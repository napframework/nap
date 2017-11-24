#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <nbitmap.h>
#include <color.h>

namespace nap
{
	/**
	 * 2D image resource that is initially empty, there is no GPU data associated with this object
	 * When initialized this object holds a set of 2D mapped pixels where every pixel value can have multiple channels
	 * This object can be declared as a resource using one of the available data and color types
	 * Every pixmap needs to have a width and height associated with it
	 * When no settings are provided the pixmap contains: 512x512, RGB8 bit pixels
	 * This object wraps a Bitmap and allocates the bitmap resource on init(). 
	 */
	class NAPAPI Pixmap : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
		 *	Supported bitmap data types
		 */
		enum class EDataType
		{
			BYTE	= 0,		//< name: Byte
			USHORT	= 2,		//< name: Short
			FLOAT	= 3			//< name: Float
		};

		/**
		 *	Supported bitmap color types
		 */
		enum class EChannels
		{
			R			= 1,	//< name: R
			RGB			= 2,	//< name: RGB
			RGBA		= 3,	//< name: RGBA
			BGR			= 4,	//< name: BGR
			BGRA		= 5		//< name: BGRA
		};

		virtual ~Pixmap();

		/**
		* The bitmap is initialized using it's associated properties. This means
		* memory is allocated based on the width, height, number of channels and type
		* of the bitmap. Failing to call init will leave the underlying bitmap empty.
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Initializes this bitmap from file. The settings associated with
		 * this bitmap will match the settings loaded from file. If you want
		 * to manually allocate pixel data call init() without a path
		 * @param path the path to the image on disk to load
		 * @param errorState contains the error if the image could not be loaded
		 * @return if the bitmap loaded successfully
		 */
		virtual bool initFromFile(const std::string& path, nap::utility::ErrorState& errorState);

		/**
		 * @return the bitmap assocoiated with this resource
		 */
		opengl::Bitmap& getBitmap()					{ return mBitmap; }

		/**
		 *	@return the bitmap associated with this resource
		 */
		const opengl::Bitmap& getBitmap() const		{ return mBitmap; }

		/**
		* @return the color of a pixel at the x and y pixel coordinates
		* The color contains a copy of the pixel values in the bitmap
		* @param x the horizontal pixel coordinate
		* @param y the vertical pixel coordinate
		*/
		std::unique_ptr<BaseColor> getColor(int x, int y) const;

		/**
		* Populates @color with the color values of a pixel.
		* This call converts the pixel data if necessary
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		*/
		void getColor(int x, int y, BaseColor& color) const;

		/**
		 * Populates @outColor with the RGB values of a pixel. 
		 * This call asserts when the bitmap doesn't have 3 channels
		 * This call does not convert incompatible types and asserts when the data types do not match
		 * @param x the horizontal coordinate of the pixel
		 * @param y the vertical coordinate of the pixel
		 * @param outColor the RGB color values of the pixel at the requested coordinates
		 */
		template<typename Type>
		void getRGBColor(int x, int y, RGBColor<Type>& outColor) const;

		/**
		* Returns the RGB values of a pixel as a color
		* This call asserts when the bitmap doesn't have 3 channels
		* This call does not convert incompatible types and asserts when the data types do not match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return the RGB values of a pixel as a color
		*/
		template<typename Type>
		RGBColor<Type> getRGBColor(int x, int y) const;

		/**
		* Populates @outColor with the RGBA values of a pixel. 
		* This call asserts when the bitmap doesn't have 4 channels
		* This call does not convert incompatible types and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param outColor the RGBA color values of the pixel at the requested coordinates
		*/
		template<typename Type>
		void getRGBAColor(int x, int y, RGBAColor<Type>& outColor) const;

		/**
		* Returns the RGBA values of a pixel as a color.
		* This call asserts when the bitmap doesn't have 4 channels
		* This call does not convert incompatible types and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return the RGBA color values of the pixel at the requested coordinates
		*/
		template<typename Type>
		RGBAColor<Type> getRGBAColor(int x, int y) const;

		/**
		 * Returns the R value (first channel) of a pixel as a color
		 * This call does not convert incompatible types and asserts when the underlying data types don't match
		 * @param x the horizontal coordinate of the pixel
		 * @param y the vertical coordinate of the pixel
		 * @param outValue the populated single channel color value 
		 */
		template<typename Type>
		void getRValue(int x, int y, RColor<Type>& outValue) const;

		int mWidth			= 512;					///< property: width of the bitmap in pixels
		int mHeight			= 512;					///< property: height of the bitmap in pixels
		EDataType mType		= EDataType::BYTE;		///< property Type: data type of the pixels in the bitmap
		EChannels mChannels	= EChannels::RGB;		///< property Channels: number and ordering of the channels in the bitmap

	protected:
		opengl::Bitmap mBitmap;
	};


	/**
	 * A pixmap resource that is loaded from file
	 * There is no GPU data associated with this object, only the pixel data extracted from the image on disk.
	 * After a successful load the bitmap properties will match that of the loaded image.  
	 */
	class NAPAPI PixmapFromFile : public Pixmap
	{
		RTTI_ENABLE(Pixmap)
	public:
		/**
		 * Loads the image pointed to by the path property
		 * Calls Pixmap::initFromFile(path, error)
		 * @param errorState contains the error when loading fails
		 * @return if loading succeeds
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::string mPath;							///< property Path: the path to the image on disk
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename Type>
	void nap::Pixmap::getRGBAColor(int x, int y, RGBAColor<Type>& outColor) const
	{
		assert(mBitmap.getNumberOfChannels() >= outColor.getNumberOfChannels());
		assert(outColor.getValueType() == RTTI_OF(Type));
		
		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		switch (mBitmap.getColorType())
		{
		case opengl::BitmapColorType::BGRA:
		{
			outColor.setValue(EColorChannel::Red, *(pixel_data + 2));
			outColor.setValue(EColorChannel::Blue, *(pixel_data + 0));
			break;
		}
		case opengl::BitmapColorType::RGBA:
		{
			outColor.setValue(EColorChannel::Red,  *(pixel_data + 0));
			outColor.setValue(EColorChannel::Blue, *(pixel_data + 2));
			break;
		}
		default:
			assert(false);
		}
		outColor.setValue(EColorChannel::Green, *(pixel_data + 1));
		outColor.setValue(EColorChannel::Alpha, *(pixel_data + 3));
	}


	template<typename Type>
	void nap::Pixmap::getRGBColor(int x, int y, RGBColor<Type>& outColor) const
	{
		assert(mBitmap.getNumberOfChannels() >= outColor.getNumberOfChannels());
		assert(outColor.getValueType() == RTTI_OF(Type));
		
		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		switch (mBitmap.getColorType())
		{
		case opengl::BitmapColorType::BGR:
		case opengl::BitmapColorType::BGRA:
		{
			outColor.setValue(EColorChannel::Red,  *(pixel_data + 2));
			outColor.setValue(EColorChannel::Blue, *(pixel_data + 0));
			break;
		}
		case opengl::BitmapColorType::RGB:
		case opengl::BitmapColorType::RGBA:
		{
			outColor.setValue(EColorChannel::Red,  *(pixel_data + 0));
			outColor.setValue(EColorChannel::Blue, *(pixel_data + 2));
			break;
		}
		default:
			assert(false);
		}
		outColor.setValue(EColorChannel::Green, *(pixel_data + 1));
	}


	template<typename Type>
	void nap::Pixmap::getRValue(int x, int y, RColor<Type>& outValue) const
	{
		assert(outValue.getValueType() == RTTI_OF(Type));
		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		outValue.setValue(EColorChannel::Red, *pixel_data);
	}


	template<typename Type>
	RGBColor<Type> nap::Pixmap::getRGBColor(int x, int y) const
	{
		RGBColor<Type> color;
		getRGBColor<Type>(x, y, color);
		return color;
	}


	template<typename Type>
	RGBAColor<Type> nap::Pixmap::getRGBAColor(int x, int y) const
	{
		RGBAColor<Type> color;
		getRGBAColor<Type>(x, y, color);
		return color;
	}
}
