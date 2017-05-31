// Local includes
#include "nbitmap.h"
#include "nglutils.h"

// External Includes
#include <assert.h>
#include <iostream>

namespace opengl
{
	/**
	 * bitmapTypeSizeMap
	 *
	 * Defines the size of every bitmap data type in bytes
	 * Important that every supported DataType has a defined size
	 */
	using BitmapTypeSizeMap = std::unordered_map<BitmapDataType, size_t>;
	static const BitmapTypeSizeMap  bitmapTypeSizeMap
	{
		{ BitmapDataType::BYTE,		sizeof(unsigned char)},
		{ BitmapDataType::FLOAT,	sizeof(float)},
		{ BitmapDataType::USHORT,	sizeof(uint16_t)}
	};


	/**
	* bitmapColorChannelMap
	*
	* Defines the number of channels associated with a certain type of color method
	*/
	using BitmapColorChannelMap = std::unordered_map<BitmapColorType, uint8_t>;
	static const BitmapColorChannelMap bitmapColorChannelMap
	{
		{ BitmapColorType::GREYSCALE,	1},
		{ BitmapColorType::INDEXED,		1},
		{ BitmapColorType::RGB,			3},
		{ BitmapColorType::RGBA,		4},
		{ BitmapColorType::BGR,			3},
		{ BitmapColorType::BGRA,		4}
	};

	// Returns the size in bytes of the bitmap type: type
	size_t getSizeOf(const BitmapDataType& type)
	{
		auto it = bitmapTypeSizeMap.find(type);
		if (it == bitmapTypeSizeMap.end())
		{
			printMessage(MessageType::ERROR, "can't figure out data size, bitmap type not registered: %d", static_cast<int8_t>(type));
			return 0;
		}
		return (*it).second;
	}


	// Returns number of channels associated with color type
	uint8_t getNumChannels(const BitmapColorType& color)
	{
		auto it = bitmapColorChannelMap.find(color);
		if (it == bitmapColorChannelMap.end())
		{
			printMessage(MessageType::ERROR, "cant figure out number of channels for color of type: %d", static_cast<int8_t>(color));
			return 0;
		}
		return (*it).second;
	}

	//////////////////////////////////////////////////////////////////////////

	// BitmapBase copy constructor
	BitmapBase::BitmapBase(const BitmapBase& other)
	{
		// Copy over settings and data ptr
		this->setSettings(other.getSettings());
		this->setData(other.getData());
	}


	// Copy assignment operator
	BitmapBase& BitmapBase::operator=(const BitmapBase& other)
	{
		// Copy over settings and data ptr
		this->setSettings(other.getSettings());
		this->setData(other.getData());
		return *this;
	}


	// Sets data associated with this bitmap, object does not own it!
	void BitmapBase::setData(const BitmapSettings& settings, void* data)
	{
		mSettings = settings;
		mData = data;
	}


	// Returns the total number of bytes associated with this image
	size_t BitmapBase::getSize()
	{
		return mSettings.mWidth * mSettings.mHeight * getSizeOf(mSettings.mDataType) * getNumChannels(mSettings.mColorType);
	}


	// Returns the total length of the pixel array, not scaled by type size
	size_t BitmapBase::getLength()
	{
		return mSettings.mWidth * mSettings.mHeight * getNumChannels(mSettings.mColorType);
	}


	// If the bitmap settings are valid
	bool BitmapSettings::isValid() const
	{
		bool has_size = (mWidth > 0 && mHeight > 0);
		return has_size && mDataType != BitmapDataType::UNKNOWN && 
			mColorType != BitmapColorType::UNKNOWN;
	}

	//////////////////////////////////////////////////////////////////////////
	// Template specializations
	//////////////////////////////////////////////////////////////////////////
	template<>
    BitmapDataType TypedBitmap<unsigned char>::getDataType() const
	{
		return BitmapDataType::BYTE;
	}

    template<>
	BitmapDataType TypedBitmap<uint16_t>::getDataType() const
	{
		return BitmapDataType::USHORT;
	}

    template<>
	BitmapDataType TypedBitmap<float>::getDataType() const
	{
		return BitmapDataType::FLOAT;
	}

	//////////////////////////////////////////////////////////////////////////
	// Bitmap
	//////////////////////////////////////////////////////////////////////////

	// Bitmap Constructor
	Bitmap::Bitmap()
	{}


	// Copy Constructor
	Bitmap::Bitmap(const Bitmap& other) : BitmapBase(other)
	{
		// Make sure data is not available
		// Otherwise copy will try to clear it
		BitmapBase::clear();

		// Copy all data from other
		if(other.hasData())
			copyData(other.getData());
	}


	// Copy operator
	Bitmap& Bitmap::operator=(const Bitmap& other)
	{
		// Copy base
		BitmapBase::operator=(other);

		// Clear existing data
		BitmapBase::clear();

		// Copy data
		if(other.hasData())
			copyData(other.getData());
		return *this;
	}


	// Bitmap Destructor
	Bitmap::~Bitmap()
	{
		clear();
	}


	// Allocates a block of memory to be associated with this bitmap
	bool Bitmap::allocateMemory()
	{
		// Ensure settings are valid
		if (!mSettings.isValid())
		{
			opengl::printMessage(MessageType::ERROR, "unable to allocate memory, bitmap settings are invalid");
			return false;
		}

		// Clear existing allocated memory
		if (BitmapBase::hasData())
		{
			printMessage(MessageType::WARNING, "bitmap already has memory allocated, clearing...");
			clear();
		}

		// Allocate new memory
		mData = malloc(BitmapBase::getSize());
		return true;
	}


	//Allocates memory based on incoming parameters
	bool Bitmap::allocateMemory(unsigned int width, unsigned int height, BitmapDataType dataType, BitmapColorType colorType)
	{
		BitmapSettings new_settings(width, height, dataType, colorType);
		if (!new_settings.isValid())
		{
			printMessage(MessageType::ERROR, "can't allocate bitmap memory, invalid settings provided");
			return false;
		}

		// Copy settings
		mSettings = new_settings;

		// Allocate memory
		return allocateMemory();
	}


	// Clears all memory associated with this bitmap
	void Bitmap::clear()
	{
		// Make sure we have something to work with
		if (mData == nullptr)
			return;

		// Free data block and call base
		free(mData);
		BitmapBase::clear();
	}


	// Copies data over in to this bitmap
	bool Bitmap::copyData(void* source)
	{
		// Make sure settings are valid
		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "can't copy pixel data, invalid bitmap settings");
			return false;
		}

		// Clear associated data
		if (BitmapBase::hasData())
			clear();

		// Allocate memory
		allocateMemory();

		// Copy
		memcpy(mData, source, BitmapBase::getSize());
		return true;
	}


	// Copies data over in to this bitmap
	bool Bitmap::copyData(unsigned int width, unsigned int height, BitmapDataType dataType, BitmapColorType colorType, void* source)
	{
		BitmapSettings new_settings(width, height, dataType, colorType);
		if (!new_settings.isValid())
		{
			printMessage(MessageType::ERROR, "can't copy bitmap data, invalid bitmap settings");
			return false;
		}
		
		// Copy settings
		mSettings = new_settings;

		// Copy
		return copyData(source);
	}
}