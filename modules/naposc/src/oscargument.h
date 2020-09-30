#pragma once

#include <nap/numeric.h>
#include <rtti/rtti.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>
#include <osc/OscOutboundPacketStream.h>
#include <sstream>

namespace nap
{
	class OSCBaseValue;
	using OSCValuePtr = std::unique_ptr<OSCBaseValue>;

	/**
	 * The OSCArgument wraps an OSCValue.
	 * This class offers some utility methods to quickly work with the most common OSC values.
	 * For other / more complex OSC Value types you can ask for the value using the get<T> function.
	 * Note that this argument owns the value.
	 */
	class NAPAPI OSCArgument final
	{
		RTTI_ENABLE()
	public:
		// Default Constructor
        OSCArgument() = default;
		OSCArgument(OSCValuePtr value);

		// Default Destructor
		~OSCArgument() = default;

		/**
		 * @return the value as type T, nullptr if the type doesn't match
		 */
		template<typename T>
		const T* get() const;

		/**
		* @return the value as type T, nullptr if the type doesn't match
		*/
		template<typename T>
		T* get();

		/**
		 *	@return the type of the OSC value
		 */
		rtti::TypeInfo getValueType() const;

	public:
		/**
		 *	@return this argument's value as a float
		 */
		float asFloat() const;

		/**
		 *	@return if this argument's value is a float
		 */
		bool isFloat() const;

		/**
		 *	@return this argument's value as an int
		 */
		int asInt() const;

		/**
		 *	@return if this argument's value is an int
		 */
		bool isInt() const;

		/**
		 *	@return this argument's value as a bool
		 */
		bool asBool() const;

		/**
		*	@return if this argument's value is a bool
		*/
		bool isBool() const;

		/**
		 *	@return this argument's value as a string
		 */
		const std::string& asString() const;

		/**
		* @return if this argument's value is a string
		*/
		bool isString() const;

		/**
		 *	@return this argument's value as a double
		 */
		double asDouble() const;

		/**
		* @return if this argument's value is a double
		*/
		bool isDouble() const;

		/**
		 *	@return this argument's value as a char
		 */
		char asChar() const;

		/**
		* @return if this argument's value is a char
		*/
		bool isChar() const;

		/**
		 *	@return if this is a null message
		 */
		bool isNil() const;

		/**
		 *	@return outValue the value as string
		 */
        std::string toString();

		/**
		* Copy is not allowed
		*/
		OSCArgument(OSCArgument&) = delete;
		OSCArgument& operator=(const OSCArgument&) = delete;

		/**
		 * Adds the value to an OSC packet that can be send over
		 * @param outPacket the packet to add the value to
		 */
		void add(osc::OutboundPacketStream& outPacket) const;

		/**
		 *	@return the size of the value type in bytes
		 */
		std::size_t size() const;

	private:
		OSCValuePtr mValue = nullptr;
	};


	//////////////////////////////////////////////////////////////////////////
	// OSC Values
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for all known OSC types
	 */
	class NAPAPI OSCBaseValue
	{
		friend OSCArgument;
		RTTI_ENABLE()
	public:
		OSCBaseValue() = default;
		virtual ~OSCBaseValue() = default;

		/**
		 * Converts the value to a string
		 * @return the value as a string
		 */
        virtual std::string toString() const = 0;
	protected:
		/**
		 * Adds the managed value to the packet
		 * @param outPacket the packet to add the value to
		 */
		virtual void add(osc::OutboundPacketStream& outPacket) const = 0;

		/**
		 * @return the size in bytes of the stored value type
		 */
		virtual std::size_t size() const = 0;
	};


	/**
	 * Simple OSC Value, for example: float, int etc.
	 */
	template<typename T>
	class OSCValue : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCValue(const T& value)  : mValue(value)								{ }
		OSCValue(const T&& value) : mValue(std::move(value))					{ }
		T mValue;
		
		/**
		 * @return the value as a string.
		 */
        virtual std::string toString() const override;
	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override;
		virtual std::size_t size() const override;
	};

	
	/**
	 *	Simple OSC String
	 */
	class NAPAPI OSCString : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCString(const std::string& string) : mString(string)						{ }
		OSCString(const std::string&& string) : mString(std::move(string))			{ }
		std::string mString;

		/**
		 * @return the message itself
		 */
        virtual std::string toString() const override;
	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override;
		virtual std::size_t size() const override;
	};


	/**
	 *	Empty (null) OSC value
	 */
	class NAPAPI OSCNil : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		/**
		 * @return 'null'
		 */
        virtual std::string toString() const override								{ return "null"; }
	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override		{ outPacket << osc::OscNil; }
		virtual std::size_t size() const override									{ return sizeof(osc::OscNil); }
	};


	/**
	 *	Time tag OSC value
	 */
	class NAPAPI OSCTimeTag : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCTimeTag(nap::uint64 timeTag) : mTimeTag(timeTag)	{ }
		nap::uint64 mTimeTag;

		/**
		 * @return the time tag as a string
		 */
        virtual std::string toString() const override								{ std::ostringstream os; os << mTimeTag; return os.str(); }
	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override		{ outPacket << osc::TimeTag(mTimeTag); }
		virtual size_t size() const override										{ return sizeof(nap::uint64); }
	};


	/**
	 *	OSC value that holds a blob of data
	 */
	class NAPAPI OSCBlob : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		/**
		 * Constructor copies the blob data block over in to mData
		 * This object ones that data, on destruction the data is deleted
		 * @param sourceData the osc data associated with the blob
		 * @param size the size of the blob in bytes
		 */
		OSCBlob(const void* sourceData, int size);
		
		/**
		 * On Destruction the blob data is deleted
		 */
		virtual ~OSCBlob();

		/**
		 * @return an unmanaged copy of the data
		 */
		void* getCopy();

		/**
		 * @return an empty string
		 */
        virtual std::string toString() const override				{ return ""; }

		// Data associated with the blob
		void* mData = nullptr;

		// Size of the blob in bytes
		int mSize = 0;

	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override;
		virtual size_t size() const override									{ return mSize; }
	};


	/**
	 *	OSC value that holds an RGBA color as 4 8 bit values (1 uint 32 bit int)
	 */
	class NAPAPI OSCColor : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCColor(nap::uint32 color) : mColor(color)								{ }

		/**
		 * @return the osc color as a string.
		 */
        virtual std::string toString() const override							{ std::ostringstream os; os << mColor; return os.str(); }

	protected:
		virtual void add(osc::OutboundPacketStream& outPacket) const override;
		virtual size_t size() const override									{ return sizeof(nap::uint32); }

	private:
		nap::uint32 mColor;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported OSC values
	//////////////////////////////////////////////////////////////////////////

	using OSCFloat =	OSCValue<float>;
	using OSCBool  =	OSCValue<bool>;
	using OSCInt =		OSCValue<int>;
	using OSCDouble =	OSCValue<double>;
	using OSCChar =		OSCValue<char>;

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	const T* nap::OSCArgument::get() const
	{
		if (!(mValue->get_type().is_derived_from(RTTI_OF(T))))
		{
			assert(false);
			return nullptr;
		}
		return static_cast<const T*>(mValue.get());
	}


	template<typename T>
	T* nap::OSCArgument::get()
	{
		if (!(mValue->get_type().is_derived_from(RTTI_OF(T))))
		{
			assert(false);
			return nullptr;
		}
		return static_cast<T*>(mValue.get());
	}


	template<typename T>
	void nap::OSCValue<T>::add(osc::OutboundPacketStream& outPacket) const
	{
		outPacket << mValue;
	}


	template<typename T>
	size_t nap::OSCValue<T>::size() const
	{
		return sizeof(T);
	}


	template<typename T>
    std::string nap::OSCValue<T>::toString() const
	{
		std::ostringstream os; 
		os << mValue; 
		return os.str();
	}
}
