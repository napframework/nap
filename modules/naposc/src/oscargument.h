#pragma once

#include <nap/configure.h>
#include <rtti/rtti.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Base class for all OSC parsed values
	 */
	class NAPAPI OSCArgument
	{
		RTTI_ENABLE()
	public:
		// Default Constructor
		OSCArgument() = default;

		// Default Destructor
		virtual ~OSCArgument() = default;

		/**
		* Copy is allowed
		*/
		OSCArgument(OSCArgument&) = default;
		OSCArgument& operator=(const OSCArgument&) = default;
	};


	/**
	 * Simple OSC value
	 */
	template<typename T>
	class NAPAPI OSCValue : public OSCArgument
	{
		RTTI_ENABLE(OSCArgument)
	public:
		OSCValue(const T& value) : mValue(value)				{ }
		T mValue;
	};


	/**
	 *	Empty (null) OSC value
	 */
	class NAPAPI OSCNil : public OSCArgument
	{
		RTTI_ENABLE(OSCArgument)
	};


	/**
	 *	Time tag OSC value
	 */
	class NAPAPI OSCTimeTag : public OSCArgument
	{
		RTTI_ENABLE(OSCArgument)
	public:
		OSCTimeTag(uint64 timeTag) : mTimeTag(timeTag)	{ }
		uint64 mTimeTag;
	};


	/**
	 *	OSC value that holds a blob of data
	 */
	class NAPAPI OSCBlob : public OSCArgument
	{
		RTTI_ENABLE(OSCArgument)
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
	class NAPAPI OSCColor : public OSCArgument
	{
		RTTI_ENABLE(OSCArgument)
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

	// Definition of an osc value as a unique ptr
	using OSCValuePtr = std::unique_ptr<nap::OSCArgument>;
}