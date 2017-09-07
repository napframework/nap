#pragma once

#include <nap/configure.h>
#include <rtti/rtti.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	class OSCBaseValue;
	using OSCValuePtr = std::unique_ptr<OSCBaseValue>;

	/**
	 * The OSCArgument wraps an OSCValue.
	 * This class offers some utility methods to quickly work with the most common OSC values
	 * For other / more complex OSC Value types you can ask for the value using the get<T> function
	 * Note that this argument owns the value.
	 */
	class NAPAPI OSCArgument
	{
		RTTI_ENABLE()
	public:
		// Default Constructor
		OSCArgument(OSCValuePtr value);

		// Default Destructor
		virtual ~OSCArgument() = default;

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
		std::string asString() const;

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
		* Copy is not allowed
		*/
		OSCArgument(OSCArgument&) = delete;
		OSCArgument& operator=(const OSCArgument&) = delete;

	private:
		OSCValuePtr mValue = nullptr;
	};


	/**
	* Base class for all known OSC types
	*/
	class NAPAPI OSCBaseValue
	{
		RTTI_ENABLE()
	public:
		OSCBaseValue() = default;
		virtual ~OSCBaseValue() = default;
	};


	/**
	 * Simple OSC Value
	 */
	template<typename T>
	class NAPAPI OSCValue : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCValue(const T& value) : mValue(value)				{ }
		T mValue;
	};


	/**
	 *	Empty (null) OSC value
	 */
	class NAPAPI OSCNil : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	};


	/**
	 *	Time tag OSC value
	 */
	class NAPAPI OSCTimeTag : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCTimeTag(uint64 timeTag) : mTimeTag(timeTag)	{ }
		uint64 mTimeTag;
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
		 *	On Destruction the blob data is deleted
		 */
		virtual ~OSCBlob();

		/**
		 * @return an unmanaged copy of the data
		 */
		void* getCopy();

		// Data associated with the blob
		void* mData = nullptr;

		// Size of the blob in bytes
		int mSize = 0;
	};


	/**
	 *	OSC value that holds an RGBA color as 4 8 bit values (1 uint 32 bit int)
	 */
	class NAPAPI OSCColor : public OSCBaseValue
	{
		RTTI_ENABLE(OSCBaseValue)
	public:
		OSCColor(uint32 color) : mColor(color)		{ }

	private:
		uint32 mColor;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported OSC values
	//////////////////////////////////////////////////////////////////////////

	using OSCFloat =	OSCValue<float>;
	using OSCBool  =	OSCValue<bool>;
	using OSCInt =		OSCValue<int>;
	using OSCDouble =	OSCValue<double>;
	using OSCChar =		OSCValue<char>;
	using OSCString =	OSCValue<std::string>;

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
}