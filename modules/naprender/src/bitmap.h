#pragma once

// External Includes
#include <FreeImage.h>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include "utility/dllexport.h"

namespace opengl
{
	/**
	* Supported bitmap data types
	* The data type is the native c++ type used per pixel per channel
	*/
	enum class BitmapDataType : uint8_t
	{
		UNKNOWN =	0,		///< unknown bitmap data type
		BYTE =		1,		///< 8 bit unsigned char
		USHORT =	2,		///< 16 bit unsigned int
		FLOAT =		3,		///< 32 bit float
	};


	/**
	* Supported bitmap color types
	* The number also determines the amount of associated bitmap channels 
	*/
	enum class BitmapColorType : uint8_t
	{
		UNKNOWN =	0,
		GREYSCALE =	1,		///< gray scale color (1 channel)
		INDEXED =	2,		///< Index lookup, 1 channel
		RGB =		3,		///< 3 channel red green blue
		RGBA =		4,		///< 4 channel red, green, blue, alpha
		BGR =		5,		///< 3 channel blue, green, red
		BGRA =		6,		///< 4 channel blue, green, red, alpha
	};

	/**
	 * Bitmap
	 *
	 * Holds a 2d bitmap (pixels) in CPU memory
	 * The bitmap does not own the data, it acts as a container
	 * Derived classes or external code needs to manage the life time of associated data
	 */
	class NAPAPI Bitmap final
	{
	public:
		// Constructor
		Bitmap() = default;
		Bitmap(unsigned int width, unsigned int height, BitmapDataType dataType, BitmapColorType colorType);

		// Destructor
		~Bitmap()
		{
			clear();
		}

		// Copy is allowed
		Bitmap(const Bitmap& other);
		Bitmap& operator=(const Bitmap& other);

		/**
		 * getData
		 *
		 * Returns the pointer to the currently associated pixel data
		 */
		void* getData() const								{ return mData; }

		bool isValid() const;

		/**
		 * hasData
		 *
		 * Returns of data is associated with this bitmap
		 * This is valid when the pointer is set
		 * When the data is deleted externally, it's up to the external code to clean up
		 */
		bool hasData() const								{ return mData != nullptr; }

		/**
		 * clear
		 *
		 * Removes data references associated with this bitmap
		 * Does NOT delete the data, that's up to external code that allocated the resources
		 */
		void clear();

		/**
		 * getSize
		 *
		 * Returns total number of bytes associated with this image
		 */
		size_t getSize();

		/**
		 * getLength
		 *
		 * Returns the total length of the pixel array, not scaled by type size
		 */
		size_t getLength();

		/**
		 * @return a pointer to the data at coordinate x, y
		 * @param x the horizontal coordinate starting at 0
		 * @param y the vertical coordinate starting at 0
		 * @return pointer to the pixel, nullptr if invalid
		 */
		void* getPixelData(unsigned int x, unsigned int y) const;

		/**
		 * @return a pointer to the pixel data at coordinate x, y cast to type T
		 * @param x the horizontal coordinate starting at 0
		 * @param y the vertical coordinate starting at 0
		 * @return pointer to the pixel, nullptr if invalid
		 * Note that it will check of the size of type T is similar to the underlying data type
		 * if not this function will return a nullptr
		 */
		template<typename T>
		T* getPixel(unsigned int x, unsigned int y) const;

		/**
		 * getWidth
		 *
		 * Returns the amount of horizontal pixels
		 */
		unsigned int getWidth() const						{ return mWidth; }

		/**
		 * getHeight
		 *
		 * Returns the amount of vertical pixels
		 */
		unsigned int getHeight() const						{ return mHeight; }

		/**
		 * getDataType
		 *
		 * Returns the data type associated with the pixel data
		 * The data type is the native c++ type used per pixel per channel
		 */
		virtual BitmapDataType getDataType() const			{ return mDataType; }

		/**
		 * getColorType
		 *
		 * Returns the bitmap's color type, UNKNOWN if not set
		 */
		BitmapColorType getColorType() const				{ return mColorType; }

		/**
		 *	@return the number of channels associated with this image, 1 for R, 4 for RGBA etc
		 */
		uint8_t getNumberOfChannels() const					{ return mNumChannels; }

		/**
		 *	@return the size in bytes of a single channel.
		 */
		uint8_t getChannelSize() const						{ return mChannelSize; }

		/**
		* allocateMemory
		* Allocates memory based on the current bitmap settings
		* If no settings have been provided previously this call will fail
		*/
		bool allocateMemory();

		/**
		* allocateMemory
		*
		* Allocates memory based on the width, height and type
		* The input values are stored in the settings container
		*/
		bool allocateMemory(unsigned int width, unsigned int height, BitmapDataType dataType, BitmapColorType colorType);

		/**
		* copyData
		*
		* Copies data from input buffer to the data buffer associated with this bitmap
		* Note that if data was set previously, the memory block pointed to will be overridden!
		* If no data is associated with the bitmap new memory is allocated to provide space for the copy operation
		* Note that this function does not check sizes, it's up to you to make sure bitmap settings are correct
		* This means that if you don't provide the right settings before a copy the application could crash
		*/
		bool copyData(void* source);

		/**
		* copyData
		*
		* Copies data from input buffer (source) to data buffer associated with this bitmap
		* Note that before the copy this call will apply the settings and allocate a new set of memory
		* This block of memory needs to match the total byte size of the source buffer
		*/
		bool copyData(unsigned int width, unsigned int height, BitmapDataType dataType, BitmapColorType colorType, void* source, unsigned int sourcePitch);

	private:
		void updateCaching();

	protected:
		void*			mData = nullptr;
		unsigned int	mWidth = 0;									///< width of the bitmap in pixels
		unsigned int	mHeight = 0;								///< height of the bitmap in pixels
		BitmapDataType	mDataType = BitmapDataType::UNKNOWN;		///< type of pixel data
		BitmapColorType mColorType = BitmapColorType::UNKNOWN;		///< color of pixel data

		size_t			mChannelSize;			///< Cached size in bytes of a single channel. This is updated when setSettings is called.
		uint8_t			mNumChannels;			///< Cached number of channels. This is updated when setSettings is called.
	};


	//////////////////////////////////////////////////////////////////////////
	// Global Functions
	//////////////////////////////////////////////////////////////////////////

		/**
	 * getSizeOf
		*
	 * Returns the size in bytes of the associated bitmap type
		*/
	size_t getSizeOf(const BitmapDataType& type);

	/**
	 * getNumChannels
	 *
	 * Returns the number of channels associated with a certain bitmap color type
	 */
	uint8_t getNumChannels(const BitmapColorType& color);


	//////////////////////////////////////////////////////////////////////////
	// Template Implementations
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* opengl::Bitmap::getPixel(unsigned int x, unsigned int y) const
	{
		assert(sizeof(T) == mChannelSize);
		return (T*)(getPixelData(x, y));
	}

} // opengl


namespace std
{
    template <>
    struct hash<opengl::BitmapColorType>
    {
        size_t operator()(const opengl::BitmapColorType& v) const
        {
            return hash<uint8_t>()(static_cast<uint8_t>(v));
        }
    };


    template <>
    struct hash<opengl::BitmapDataType>
    {
        size_t operator()(const opengl::BitmapDataType& v) const
        {
            return hash<uint8_t>()(static_cast<uint8_t>(v));
        }
    };
}



