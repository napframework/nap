#include "bitmaputils.h"
#include "nglutils.h"
#include "utility/errorstate.h"
#include <assert.h>
#include <algorithm>
#include "ntexture2d.h"

namespace nap
{
	/**
	* freeImageTypeMap
	*
	* Holds all bitmap to free image type conversions
	* Note that every 8 bit image is a FIT_BITMAP in freeImage
	*/
	using FreeImageTypeMap = std::unordered_map<Pixmap::EDataType, std::vector<FREE_IMAGE_TYPE>>;
	static const FreeImageTypeMap freeImageTypeMap =
	{
		{ Pixmap::EDataType::BYTE,		{ FIT_BITMAP }},
		{ Pixmap::EDataType::USHORT,	{ FIT_UINT16, FIT_RGB16, FIT_RGBA16}},
		{ Pixmap::EDataType::FLOAT,		{ FIT_FLOAT, FIT_RGBF, FIT_RGBAF}}
	};


	/**
	 * freeImageColorMap
	 *
	 * Holds all bitmap color to free image color type conversions
	 * If there are multiple vector entries, the second entry is always the little endian version
	 * TODO: Figure out something nicer for this lookup
	 */
	using FreeImageColorMap = std::unordered_map<FREE_IMAGE_COLOR_TYPE, Pixmap::EChannels>;
	static const FreeImageColorMap freeImageColorMap
	{
		{ FIC_MINISBLACK,	Pixmap::EChannels::R },
		{ FIC_RGB,			Pixmap::EChannels::RGB },
		{ FIC_RGBALPHA,		Pixmap::EChannels::RGBA }
	};


	//Returns the associated Bitmap Data Type for the queried free image type
	Pixmap::EDataType getBitmapType(FREE_IMAGE_TYPE type)
	{
		auto found_it = std::find_if(freeImageTypeMap.begin(), freeImageTypeMap.end(), [&](const auto& value)
		{
			bool found(false);
			for (const auto& v : value.second)
			{
				if (v == type)
				{
					found = true;
					break;
				}
			}
			return found;
		});

		assert(found_it != freeImageTypeMap.end());

		return (*found_it).first;
	}


	Pixmap::EDataType getBitmapType(GLenum gltype)
	{
		for (const auto& type : getGLTypeMap())
		{
			if (type.second == gltype)
				return type.first;
		}
		assert(false);
		return Pixmap::EDataType::BYTE;
	}


	Pixmap::EChannels getColorType(GLenum format)
	{
		for (const auto& colorformat : getGLFormatMap())
		{
			if (colorformat.second == format)
				return colorformat.first;
		}
		assert(false);
		return Pixmap::EChannels::RGB;
	}


	/**
	 * getColorType
	 *
	 * Figures out what type of color belongs to the free image color type
	 * A bitmap loaded with free image can have a different channel ordering based on the endianness of the system
	 * For that reason it's important to know the image's data type before returning the color type to use
	 * If the free image bitmap is of type FIT_BITMAP and has an RGB or RGBA color scheme
	 * The channels might be ordered differently base don little / big endian
	 * This function compensates for that ordering and returns the correct color type based on the difference in architecture
	 */
	Pixmap::EChannels getColorType(FREE_IMAGE_COLOR_TYPE colorType, FREE_IMAGE_TYPE dataType)
	{
		// Find based on color key
		auto it = freeImageColorMap.find(colorType);

		// If no match was found return
		if (it == freeImageColorMap.end())
		{
			assert(false);
			return Pixmap::EChannels::RGB;
		}

		// If we're dealing with an rgb or rgba map and a bitmap
		// The endian of the loaded free image map becomes important
		// If so we might have to swap the red and blue channels regarding the interal color representation
		bool swap = (dataType == FREE_IMAGE_TYPE::FIT_BITMAP && FI_RGBA_RED == 2);

		// Based on type, swap if necessary
		Pixmap::EChannels return_type = it->second;
		switch (it->second)
		{
		case Pixmap::EChannels::RGB:
			return_type = swap ? Pixmap::EChannels::BGR : Pixmap::EChannels::RGB;
			break;
		case Pixmap::EChannels::RGBA:
			return_type = swap ? Pixmap::EChannels::BGRA : Pixmap::EChannels::RGBA;
			break;
		default:
			break;
		}

		// Swapping is of no importance
		return return_type;
	}

/*
	// Loads bitmap from image file path
	Bitmap* loadBitmap(const std::string& imgPath, nap::utility::ErrorState& errorState)
	{
		// Create a new bitmap
		opengl::Bitmap* nbitmap = new Bitmap();
		if (!loadBitmap(*nbitmap, imgPath, errorState))
		{
			delete nbitmap;
			return nullptr;
		}
		return nbitmap;
	}


	// loads a bitmap from file
	bool loadBitmap(Bitmap& bitmap, const std::string& imgPath, nap::utility::ErrorState& errorState)
	{
		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(imgPath.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format of file: %s", imgPath.c_str()))
			return false;

		// Load
		FIBITMAP* fi_bitmap = FreeImage_Load(fi_img_format, imgPath.c_str());
		if (!errorState.check(fi_bitmap != nullptr, "Unable to load bitmap: %s", imgPath.c_str()))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		// Get associated bitmap type for free image type
		FREE_IMAGE_TYPE fi_bitmap_type = FreeImage_GetImageType(fi_bitmap);
		BitmapDataType bitmap_type = getBitmapType(fi_bitmap_type);
		if (!errorState.check(bitmap_type != BitmapDataType::UNKNOWN, "Unable to find matching bitmap type for free image type: %d", static_cast<int>(fi_bitmap_type)))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		// Get color type
		FREE_IMAGE_COLOR_TYPE fi_bitmap_color_type = FreeImage_GetColorType(fi_bitmap);
		BitmapColorType bitmap_color = getColorType(fi_bitmap_color_type, fi_bitmap_type);
		if (!errorState.check(bitmap_color != BitmapColorType::UNKNOWN, "Unable to find matching bitmap color type for free image type: %d", static_cast<int>(fi_bitmap_color_type)))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}
		
		// Copy free image data in our own bitmap container
		uint8_t* fi_data = FreeImage_GetBits(fi_bitmap);		
		assert(fi_data != nullptr);		
		bool success = bitmap.copyData(FreeImage_GetWidth(fi_bitmap), FreeImage_GetHeight(fi_bitmap), bitmap_type, bitmap_color, fi_data, FreeImage_GetPitch(fi_bitmap));
		assert(success);

		FreeImage_Unload(fi_bitmap);
		return success;
	}
	*/

	/**
	 * GLTex2DInternalFormatOptions
	 *
	 * Holds the available format options for a 2D opengl texture
	 * For more info on texture formats check: https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
	 */
	struct GLTex2DInternalFormatOption
	{
		// Constructor
		GLTex2DInternalFormatOption()  = default;
		GLTex2DInternalFormatOption(GLint glUncompressedType, GLint glPreferredCompression, const std::vector<GLint>& glCompressedTypes) :
			mUncompressedInternalFormat(glUncompressedType),
			mPreferredInternalCompression(glPreferredCompression),
			mCompressedInternalFormats(glCompressedTypes)			{ }

		~GLTex2DInternalFormatOption() = default;

		GLint				mUncompressedInternalFormat		= GL_INVALID_VALUE;		//< GL's matching uncompressed internal format
        GLint				mPreferredInternalCompression	= GL_INVALID_VALUE;		//< GL's preferred internal compression
		std::vector<GLint>	mCompressedInternalFormats;								//< GL's matching compressed  internal formats

		// Returns if this option supports compression
		bool supportsCompression() const					{ return mPreferredInternalCompression != GL_INVALID_VALUE; }
	};


	/**
	 * tex2DInternalFormatBindings
	 *
	 * Holds all texture 2D available bitmap to GL internal format options
	 * Note that we do not support indexed colors right now, as we need to implement color tables
	 */
	using GLTex2DFormatBindingMap = std::unordered_map<Pixmap::EChannels, GLTex2DInternalFormatOption>;
	static const GLTex2DFormatBindingMap& getTex2DformatBindings()
	{
		static GLTex2DFormatBindingMap tex2DInternalFormatBindings;
		if (tex2DInternalFormatBindings.empty())
		{
//			tex2DInternalFormatBindings[Pixmap::EChannels::GREYSCALE] =	{ GL_RED,		GL_COMPRESSED_RED_RGTC1_EXT,		{ GL_COMPRESSED_RED,		GL_COMPRESSED_RED_RGTC1_EXT} };
			tex2DInternalFormatBindings[Pixmap::EChannels::RGB] =		{ GL_RGB,		GL_COMPRESSED_RGB_S3TC_DXT1_EXT,	{ GL_COMPRESSED_RGB,		GL_COMPRESSED_RGB_S3TC_DXT1_EXT } };
			tex2DInternalFormatBindings[Pixmap::EChannels::RGBA] =		{ GL_RGBA,		GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,	{ GL_COMPRESSED_RGBA,		GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT } };
			tex2DInternalFormatBindings[Pixmap::EChannels::BGR] =		tex2DInternalFormatBindings[Pixmap::EChannels::RGB];
			tex2DInternalFormatBindings[Pixmap::EChannels::BGRA] =		tex2DInternalFormatBindings[Pixmap::EChannels::RGBA];
		}
		return tex2DInternalFormatBindings;
	}


	/**
	* getInternalFormatOptions
	*
	* Helper function to retrieve internal format options based on color type
	* Returns null if not found
	*/
	static const GLTex2DInternalFormatOption* getInternalFormatOption(Pixmap::EChannels type)
	{
		const GLTex2DFormatBindingMap& all_options = getTex2DformatBindings();
		auto it = getTex2DformatBindings().find(type);
		if (it == all_options.end())
		{
			//printMessage(MessageType::ERROR, "unable to find GL texture 2d internal format options for bitmap color of type: %d", static_cast<uint8_t>(type));
			return nullptr;
		}
		return &(it->second);
	}

	//////////////////////////////////////////////////////////////////////////

	/**
	 * openGLFormatMap 
	 *
	 * Maps bitmap color types to supported GL formats
	 */
	static const OpenGLFormatMap openGLFormatMap = 
	{
//		{ Pixmap::EChannels::GREYSCALE,	GL_RED  },
		{ Pixmap::EChannels::RGB,		GL_RGB  },
		{ Pixmap::EChannels::RGBA,		GL_RGBA },
		{ Pixmap::EChannels::BGR,		GL_BGR  },
		{ Pixmap::EChannels::BGRA,		GL_BGRA }
	};

	//////////////////////////////////////////////////////////////////////////


	/**
	* openGLTypeMap
	*
	* Maps bitmap types to opengl types
	*/
	static const OpenGLTypeMap openGLTypeMap =
	{
		{ Pixmap::EDataType::BYTE,		GL_UNSIGNED_BYTE },
		{ Pixmap::EDataType::FLOAT,		GL_FLOAT },
		{ Pixmap::EDataType::USHORT,	GL_UNSIGNED_SHORT }
	};


	//////////////////////////////////////////////////////////////////////////


	// Returns the associated OpenGL system type based on the Bitmap's data type
	GLenum getGLType(Pixmap::EDataType type)
	{
		auto it = openGLTypeMap.find(type);
		return it == openGLTypeMap.end() ? GL_INVALID_ENUM : it->second;
	}


	// the GL associated internal format associated with the BitmapColorType
	GLint getGLInternalFormat(Pixmap::EChannels type, bool compressed /*= true*/)
	{
		// Get options and check if available
		const GLTex2DInternalFormatOption* options = getInternalFormatOption(type);
		if (options == nullptr)
			return GL_INVALID_VALUE;
	
		// Return correct internal format
		if (options->supportsCompression() && compressed)
			return options->mPreferredInternalCompression;
		return options->mUncompressedInternalFormat;	
	}


	// Returns the GL associated format associated with the bitmap's color type
	GLenum getGLFormat(Pixmap::EChannels type)
	{
		auto it = openGLFormatMap.find(type);
		if (it == openGLFormatMap.end())
		{
//			printMessage(MessageType::ERROR, "unable to find matching gl format for bitmap color of type: %d", static_cast<uint8_t>(type));
			return GL_INVALID_ENUM;
		}
		return it->second;
	}

	
// 	// Helper function to check bitmap validity
// 	bool checkBitmap(const Pixmap& pixmap)
// 	{
// 		if (!pixmap.isValid())
// 		{
// 			printMessage(MessageType::ERROR, "unable to set texture from bitmap, invalid bitmap settings");
// 			return false;
// 		}
// 
// 		if (!bitmap.hasData())
// 		{
// 			printMessage(MessageType::ERROR, "unable to set texture from bitmap, bitmap has no associated data");
// 			return false;
// 		}
// 		return true;
// 	}


	// Populates a Texture2D object with settings matching the bitmap
	bool getSettingsFromBitmap(const Pixmap& pixmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState)
	{
		//assert(checkBitmap(bitmap));

		// Fetch matching values
		GLint internal_format = getGLInternalFormat(pixmap.getChannels(), compress);
		if (!errorState.check(internal_format != GL_INVALID_VALUE, "Unable to determine internal format from bitmap"))
			return false;

		GLenum format = getGLFormat(pixmap.getChannels());
		if (!errorState.check(format != GL_INVALID_ENUM, "Unable to determine format from bitmap"))
			return false;

		GLenum type = getGLType(pixmap.getDataType());
		if (!errorState.check(type != GL_INVALID_ENUM, "Unable to determine texture type from bitmap"))
			return false;

		// Populate settings with fetched values from bitmap
		settings.mInternalFormat = internal_format;
		settings.mFormat = format;
		settings.mType = type;
		settings.mWidth = pixmap.getWidth();
		settings.mHeight = pixmap.getHeight();

		return true;
	}


	const OpenGLTypeMap& getGLTypeMap()
	{
		return openGLTypeMap;
	}


	const OpenGLFormatMap& getGLFormatMap()
	{
		return openGLFormatMap;
	}
}