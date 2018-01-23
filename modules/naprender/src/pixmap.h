#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <bitmap.h>
#include <color.h>

namespace nap
{
	class BaseTexture2D;

	/**
	 * 2D image resource that is initially empty, there is no GPU data associated with this object
	 * When initialized this object holds a set of 2D mapped pixels where every pixel value can have multiple channels
	 * This object can be declared as a resource using one of the available data and color types
	 * Every pixmap needs to have a width and height associated with it
	 * When no settings are provided the pixmap contains: 512x512, RGB8 bit pixels
	 * This object wraps a Bitmap and allocates the bitmap resource on init()
	 * The properties associated with the pixmap are set when initialized from texture or file
	 */
	class NAPAPI Pixmap : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
		 *	Supported bitmap data types
		 */
		enum class EDataType : int
		{
			BYTE	= 0,		//< name: Byte
			USHORT	= 2,		//< name: Short
			FLOAT	= 3			//< name: Float
		};

		/**
		 *	Supported bitmap color types
		 */
		enum class EChannels : int
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
		 * to manually allocate pixel data call init() without a path.
		 * The pixel data associated with the image @path is copied over
		 * @param path the path to the image on disk to load
		 * @param errorState contains the error if the image could not be loaded
		 * @return if the bitmap loaded successfully
		 */
		virtual bool initFromFile(const std::string& path, nap::utility::ErrorState& errorState);

		/**
		 * Initializes this bitmap from a 2D texture. 
		 * The settings associated with this bitmap will match the settings of the 2D texture.
		 * Memory is allocated but the GPU pixel data is NOT copied over
		 * @param texture the GPU texture to initialize this pixmap from
		 */
		void initFromTexture(const nap::BaseTexture2D& texture);

		/**
		 * @return the type of color associated with this pixmap
		 */
		rtti::TypeInfo getColorType() const									{ return mColorType; }

		/**
		 * @return if the pixmap is empty
		 * This is the case when the bitmap has not been initialized
		 */
		bool empty() const													{ return !(mBitmap.hasData()); }

		/**
		 * @return the width of the pixmap, 0 when not initialized
		 */
		int getWidth() const												{ return mBitmap.getWidth(); }

		/**
		 *	@return the height of the pixmap, 0 when not initialized
		 */
		int getHeight() const												{ return mBitmap.getHeight(); }
 
		/**
		 * @return the bitmap associated with this resource
		 */
		opengl::Bitmap& getBitmap()											{ return mBitmap; }

		/**
		 *	@return the bitmap associated with this resource
		 */
		const opengl::Bitmap& getBitmap() const								{ return mBitmap; }

		/**
		 *	@return number of color channels associated with this pixmap
		 */
		int getNumberOfChannels() const										{ return mBitmap.getNumberOfChannels(); }

		/**
		 * @return a pointer to the underlying data in memory
		 */
		void* getData()														{ return mBitmap.getData(); }

		/**
		* Creates a color that is compatible with the data stored in this pixmap
		* This is a utility function that works in conjunction with getPixel() and setPixel().  
		* Making the pixel once before iterating over all the values in this map avoids unnecessary allocations
		* @return a new pixel as a color that matches the amount of channels and data type of this pixmap
		*/
		std::unique_ptr<BaseColor> makePixel() const;

		/**
		* Retrieves the color of a pixel at the x and y pixel coordinates
		* The color is a copy of the pixel values in the bitmap. The result is stored in outPixel.
		* outPixel needs to be created using makePixel(), this ensures the right number of channels and pixmap value type of the color
		* outPixel needs to own it's color data and can't point to values in memory
		* This call does not convert outPixel if the types don't match. In that case this call will assert
		* To convert the fetched data call .convert() on outPixel
		* @param x the horizontal pixel coordinate
		* @param y the vertical pixel coordinate
		* @param outPixel the pixel created using makePixel()
		*/
		void getPixel(int x, int y, BaseColor& outPixel) const;

		/**
		* return a color of type T with the color values of a pixel.
		* This call converts the pixel data if necessary. 
		* Note that this call can be slow when iterating over the bitmap!
		* Use the makePixel / getPixel combination above for faster results
		* Use this call to get a copy of the color values in the desired color format T where
		* T can not be a color that points to external value in memory, ie: RGBColorData8 etc.
		* Valid values for T are RGBColor8, RColorFloat etc.
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param outColor holds the converted pixel colors
		*/
		template<typename T>
		T getPixel(int x, int y) const;

		/**
		 * Sets the color of a pixel at the x and y pixel coordinates
		 * This call does not convert the color if the value types don't match, best to convert the color client side.
		 * It's allowed to give an input color can that has less color channels than a pixel in the pixmap.
		 * This means that an RGB color can be set to a pixel of an RGBA pixmap
		 * To ensure matching data use makePixel() to create a pixel that is compatible with this pixmap
		 * You can use the color conversion methods to convert any color into this pixmap's color space
		 * This call asserts when the color types don't match and when the input color doesn't own it's data: points to values in memory
		 * @param x the horizontal pixel coordinate
		 * @param y the vertical pixel coordinate
		 * @param color the new pixel color
		*/
		void setPixel(int x, int y, const BaseColor& color);

		/**
		 * Sets the color of a pixel at the x and y pixel coordinates
		 * This call converts the incoming color when the underlying data types do not match
		 * The given color can also point to values in memory, ie: the color doesn't own it's data
		 * but the data it owns is copied over.
		 * It's not recommended to use this call in a loop when you know it needs to convert the color
		 * It's allowed to give an input color can that has less color channels than a pixel in the pixmap.
		 * This means that an RGB color can be set to a pixel of an RGBA pixmap
		 * @param x the horizontal pixel coordinate
		 * @param y the horizontal pixel coordinate
		 * @param color the new pixel color
		 */
		template<typename T>
		void setPixelColor(int x, int y, const T& color);

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
		* Returns a copy of the RGBA values of a pixel as a color.
		* This call asserts when the bitmap doesn't have 4 channels
		* This call does not convert incompatible types and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return the RGBA color values of the pixel at the requested coordinates
		*/
		template<typename Type>
		RGBAColor<Type> getRGBAColor(int x, int y) const;

		/**
		 * Populates @outValue with the color value @channel
		 * This call does not convert incompatible types and asserts when the underlying data types don't match
		 * @param x the horizontal coordinate of the pixel
		 * @param y the vertical coordinate of the pixel
		 * @param channel the color channel to get the value for
		 * @param outValue copy of the color value associated with @channel
		 */
		template<typename Type>
		void getColorValue(int x, int y, nap::EColorChannel channel, RColor<Type>& outValue) const;

		/**
		* Returns a copy of the color value @channel
		* This call does not convert incompatible types and asserts when the underlying data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param channel the color channel to get the value for
		* @return copy of the color value associated with @channel
		*/
		template<typename Type>
		RColor<Type> getColorValue(int x, int y, nap::EColorChannel channel) const;

		/**
		 * These properties are read when initializing the pixmap as a resource
		 * These properties are set  when initializing the pixmap from file or texture
		 */
		int mWidth					= 512;						///< property: width of the bitmap in pixels
		int mHeight					= 512;						///< property: height of the bitmap in pixels
		EDataType mType				= EDataType::BYTE;			///< property Type: data type of the pixels in the bitmap
		EChannels mChannels			= EChannels::RGB;			///< property Channels: number and ordering of the channels in the bitmap

	protected:
		opengl::Bitmap mBitmap;

	private:
		/**
		 * Helper function that ensures the pixmap settings are in sync with the settings associated with it's bitmap;
		 */
		void applySettingsFromBitmap();
		
		/**
		 * Figures out this bitmap's type of color and stores it for further use
		 */
		void onInit();

		/**
		* Populates @outColorData with the memory addresses of the RGB pixel values
		* This call is useful to retrieve the memory location of a pixel's color values in a bitmap
		* This call asserts when the bitmap doesn't have 3 channels
		* This call does not perform any type of conversion and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param outColorData the RGB memory addresses of the pixel at the requested coordinates
		*/
		template<typename Type>
		void getRGBColorData(int x, int y, RGBColor<Type*>& outColorData) const;

		/**
		* Populates @outColorData with the memory addresses of the RGBA pixel values
		* This call is useful to retrieve the memory location of a pixel's color values in a bitmap
		* This call asserts when the bitmap doesn't have 4 channels
		* This call does not perform any type of conversion and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param outColorData the RGBA memory addresses of the pixel at the requested coordinates
		*/
		template<typename Type>
		void getRGBAColorData(int x, int y, RGBAColor<Type*>& outColorData) const;

		/**
		* Returns the memory addresses of the RGBA pixel values
		* This call is useful to retrieve the memory location of a pixel's color values in a bitmap
		* This call asserts when the bitmap doesn't have 4 channels
		* This call does not perform any type of conversion and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return the RGBA memory addresses of the pixel at the requested coordinates
		*/
		template<typename Type>
		RGBAColor<Type*> getRGBAColorData(int x, int y) const;

		/**
		* Returns the memory addresses of the RGB pixel values
		* This call is useful to retrieve the memory location of a pixel's color values in a bitmap
		* This call asserts when the bitmap doesn't have 3 channels
		* This call does not perform any type of conversion and asserts when the data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return the RGB memory addresses of the pixel colors at the requested coordinates
		*/
		template<typename Type>
		RGBColor<Type*> getRGBColorData(int x, int y) const;

		/**
		* Populates @outValue with the address of the color value @channel
		* This call is useful to retrieve the memory location of a pixel's color channel in a bitmap
		* This call does not convert incompatible types and asserts when the underlying data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @param outValue pointer to the color data associated with the @channel
		*/
		template<typename Type>
		void getColorValueData(int x, int y, nap::EColorChannel channel, RColor<Type*>& outValue) const;

		/**
		* returns a pointer to the address of the color value @channel
		* This call is useful to retrieve the memory location of a pixel's color channel in a bitmap
		* This call does not convert incompatible types and asserts when the underlying data types don't match
		* @param x the horizontal coordinate of the pixel
		* @param y the vertical coordinate of the pixel
		* @return pointer to the color data associated with the value @channel
		*/
		template<typename Type>
		RColor<Type*> getColorValueData(int x, int y, nap::EColorChannel channel) const;

		/**
		* Sets the data associated with this pixmap at the x and y coordinates to color
		* @param x the horizontal pixel coordinate
		* @param y the vertical pixel coordinate
		* @param color the new pixel color
		*/
		template<typename T>
		void setPixelData(int x, int y, const nap::BaseColor& color);

		rtti::TypeInfo mColorType = rtti::TypeInfo::empty();	///< Type of color associated with this pixmap (RGBA8, R16 etc.)
		rtti::TypeInfo mValueType = rtti::TypeInfo::empty();	///< Contained value type of the color (byte, float etc.)
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
		RGBAColor<Type*> color_data;
		getRGBAColorData<Type>(x, y, color_data);

		outColor.setValue(EColorChannel::Red,   *(color_data.getValue(EColorChannel::Red)));
		outColor.setValue(EColorChannel::Green, *(color_data.getValue(EColorChannel::Green)));
		outColor.setValue(EColorChannel::Blue,  *(color_data.getValue(EColorChannel::Blue)));
		outColor.setValue(EColorChannel::Alpha, *(color_data.getValue(EColorChannel::Alpha)));
	}


	template<typename Type>
	void nap::Pixmap::getRGBColor(int x, int y, RGBColor<Type>& outColor) const
	{
		RGBColor<Type*> color_data;
		getRGBColorData<Type>(x, y, color_data);

		outColor.setValue(EColorChannel::Red,   *(color_data.getValue(EColorChannel::Red)));
		outColor.setValue(EColorChannel::Green, *(color_data.getValue(EColorChannel::Green)));
		outColor.setValue(EColorChannel::Blue,  *(color_data.getValue(EColorChannel::Blue)));
	}


	template<typename Type>
	void nap::Pixmap::getColorValue(int x, int y, nap::EColorChannel channel, RColor<Type>& outValue) const
	{
		RColor<Type*> color_value;
		getColorValueData<Type>(x, y, channel, color_value);
		outValue.setValue(EColorChannel::Red, *(color_value.getValue(EColorChannel::Red)));
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


	template<typename Type>
	void nap::Pixmap::getRGBAColorData(int x, int y, RGBAColor<Type*>& outColor) const
	{
		assert(mBitmap.getNumberOfChannels() >= outColor.getNumberOfChannels());
		assert(outColor.getValueType() == RTTI_OF(Type));
		assert(mBitmap.hasData());

		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		switch (mBitmap.getColorType())
		{
		case opengl::BitmapColorType::BGRA:
		{
			outColor.setValue(EColorChannel::Red,  pixel_data + 2);
			outColor.setValue(EColorChannel::Blue, pixel_data + 0);
			break;
		}
		case opengl::BitmapColorType::RGBA:
		{
			outColor.setValue(EColorChannel::Red,  pixel_data + 0);
			outColor.setValue(EColorChannel::Blue, pixel_data + 2);
			break;
		}
		default:
			assert(false);
		}
		outColor.setValue(EColorChannel::Green, pixel_data + 1);
		outColor.setValue(EColorChannel::Alpha, pixel_data + 3);
	}


	template<typename Type>
	RGBAColor<Type*> nap::Pixmap::getRGBAColorData(int x, int y) const
	{
		RGBAColor<Type*> rcolor;
		getRGBAColorData<Type>(x, y, rcolor);
		return rcolor;
	}


	template<typename Type>
	void nap::Pixmap::getRGBColorData(int x, int y, RGBColor<Type*>& outColor) const
	{
		assert(mBitmap.getNumberOfChannels() >= outColor.getNumberOfChannels());
		assert(outColor.getValueType() == RTTI_OF(Type));
		assert(mBitmap.hasData());

		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		switch (mBitmap.getColorType())
		{
		case opengl::BitmapColorType::BGR:
		case opengl::BitmapColorType::BGRA:
		{
			outColor.setValue(EColorChannel::Red,  pixel_data + 2);
			outColor.setValue(EColorChannel::Blue, pixel_data + 0);
			break;
		}
		case opengl::BitmapColorType::RGB:
		case opengl::BitmapColorType::RGBA:
		{
			outColor.setValue(EColorChannel::Red,  pixel_data + 0);
			outColor.setValue(EColorChannel::Blue, pixel_data + 2);
			break;
		}
		default:
			assert(false);
		}
		outColor.setValue(EColorChannel::Green,	pixel_data + 1);
	}


	template<typename Type>
	RGBColor<Type*> nap::Pixmap::getRGBColorData(int x, int y) const
	{
		RGBColor<Type*> rcolor;
		getRGBColorData<Type>(x, y, rcolor);
		return rcolor;
	}


	template<typename Type>
	void nap::Pixmap::getColorValueData(int x, int y, nap::EColorChannel channel, RColor<Type*>& outValue) const
	{
		assert(outValue.getValueType() == RTTI_OF(Type));
		assert(mBitmap.hasData());
		assert(static_cast<int>(channel) < mBitmap.getNumberOfChannels());

		int idx = static_cast<int>(channel);
		switch (mBitmap.getColorType())
		{
			case opengl::BitmapColorType::BGR:
			case opengl::BitmapColorType::BGRA:
			{
				idx = channel == EColorChannel::Red  ? 2 : 
					  channel == EColorChannel::Blue ? 0 : idx;
				break;
			}
			default:
				break;
		}
		Type* pixel_data = mBitmap.getPixel<Type>(x, y);
		outValue.setValue(EColorChannel::Red, pixel_data + idx);
	}


	template<typename Type> 
	RColor<Type> nap::Pixmap::getColorValue(int x, int y, nap::EColorChannel channel) const
	{
		RColor<Type> rvalue;
		getColorValue<Type>(x, y, channel, rvalue);
		return rvalue;
	}


	template<typename Type>
	RColor<Type*> nap::Pixmap::getColorValueData(int x, int y, nap::EColorChannel channel) const
	{
		RColor<Type*> rvalue;
		getColorValueData(x, y, channel, rvalue);
		return rvalue;
	}


	template<typename T>
	T nap::Pixmap::getPixel(int x, int y) const
	{
		// Create a pixel and fill it
		std::unique_ptr<BaseColor> pixel = makePixel();
		getPixel(x, y, *pixel);

		// If they are of the same type it's safe to cast and return
		if (pixel->get_type().is_derived_from(RTTI_OF(T)))
		{
			return *(static_cast<T*>(pixel.get()));
		}
		// Otherwise we need to convert
		return pixel->convert<T>();
	}


	template<typename T>
	void nap::Pixmap::setPixelColor(int x, int y, const T& color)
	{
		if (color.getValueType() == mValueType && !(color.isPointer()))
		{
			setPixel(x, y, color);
			return;
		}

		std::unique_ptr<BaseColor> new_pixel = makePixel();
		const BaseColor& source_color = static_cast<const BaseColor&>(color);
		source_color.convert(*new_pixel);
		setPixel(x, y, *new_pixel);
	}


	template<typename T>
	void nap::Pixmap::setPixelData(int x, int y, const nap::BaseColor& color)
	{
		// We need to make sure that the underlying color value types match
		// The incoming color can't be a pointer
		assert(mValueType == color.getValueType());
		assert(!color.isPointer());

		switch (color.getNumberOfChannels())
		{
		case 1:
		{
			const nap::RColor<T>* clr = static_cast<const nap::RColor<T>*>(&color);
			nap::RColor<T*> data = this->getColorValueData<T>(x, y, nap::EColorChannel::Red);
			(*data.getRed())	= clr->getRed();
			break;
		}
		case 3:
		{
			const nap::RGBColor<T>* clr = static_cast<const nap::RGBColor<T>*>(&color);
			nap::RGBColor<T*> data = this->getRGBColorData<T>(x, y);
			(*data.getRed())	= clr->getRed();
			(*data.getGreen())	= clr->getGreen();
			(*data.getBlue())	= clr->getBlue();
			break;
		}
		case 4:
		{
			const nap::RGBAColor<T>* clr = static_cast<const nap::RGBAColor<T>*>(&color);
			nap::RGBAColor<T*> data = this->getRGBAColorData<T>(x, y);
			(*data.getRed())	= clr->getRed();
			(*data.getGreen())	= clr->getGreen();
			(*data.getBlue())	= clr->getBlue();
			(*data.getAlpha())	= clr->getAlpha();
			break;
		}
		default:
			assert(false);
		}
	}
}

namespace std
{
	template <>
	struct hash<nap::Pixmap::EChannels>
	{
		size_t operator()(const nap::Pixmap::EChannels& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};


	template <>
	struct hash<nap::Pixmap::EDataType>
	{
		size_t operator()(const nap::Pixmap::EDataType& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};
}
