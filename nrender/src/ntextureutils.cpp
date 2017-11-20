#include "ntextureutils.h"
#include "ntexture2d.h"
#include "nglutils.h"
#include "assert.h"
#include "utility/errorstate.h"

namespace opengl
{
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
	 * TODO: NEEDS A MUTEX, MULTIPLE THREADS COULD QUERY THE DATA AT THE SAME TIME, CORRUPTING IT
	 */
	using GLTex2DFormatBindingMap = std::unordered_map<BitmapColorType, GLTex2DInternalFormatOption>;
	static const GLTex2DFormatBindingMap& getTex2DformatBindings()
	{
		static GLTex2DFormatBindingMap tex2DInternalFormatBindings;
		if (tex2DInternalFormatBindings.empty())
		{
			tex2DInternalFormatBindings[BitmapColorType::GREYSCALE] =	{ GL_RED,		GL_COMPRESSED_RED_RGTC1_EXT,		{ GL_COMPRESSED_RED,		GL_COMPRESSED_RED_RGTC1_EXT} };
			tex2DInternalFormatBindings[BitmapColorType::RGB] =			{ GL_RGB,		GL_COMPRESSED_RGB_S3TC_DXT1_EXT,	{ GL_COMPRESSED_RGB,		GL_COMPRESSED_RGB_S3TC_DXT1_EXT } };
			tex2DInternalFormatBindings[BitmapColorType::RGBA] =		{ GL_RGBA,		GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,	{ GL_COMPRESSED_RGBA,		GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT } };
			tex2DInternalFormatBindings[BitmapColorType::BGR] =			tex2DInternalFormatBindings[BitmapColorType::RGB];
			tex2DInternalFormatBindings[BitmapColorType::BGRA] =		tex2DInternalFormatBindings[BitmapColorType::RGBA];
		}
		return tex2DInternalFormatBindings;
	}


	/**
	* getInternalFormatOptions
	*
	* Helper function to retrieve internal format options based on color type
	* Returns null if not found
	*/
	static const GLTex2DInternalFormatOption* getInternalFormatOption(BitmapColorType type)
	{
		const GLTex2DFormatBindingMap& all_options = getTex2DformatBindings();
		auto it = getTex2DformatBindings().find(type);
		if (it == all_options.end())
		{
			printMessage(MessageType::ERROR, "unable to find GL texture 2d internal format options for bitmap color of type: %d", static_cast<uint8_t>(type));
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
		{ BitmapColorType::GREYSCALE,	GL_RED  },
		{ BitmapColorType::RGB,			GL_RGB  },
		{ BitmapColorType::RGBA,		GL_RGBA },
		{ BitmapColorType::BGR,			GL_BGR  },
		{ BitmapColorType::BGRA,		GL_BGRA }
	};

	//////////////////////////////////////////////////////////////////////////


	/**
	* openGLTypeMap
	*
	* Maps bitmap types to opengl types
	*/
	static const OpenGLTypeMap openGLTypeMap =
	{
		{ BitmapDataType::BYTE,		GL_UNSIGNED_BYTE },
		{ BitmapDataType::FLOAT,	GL_FLOAT },
		{ BitmapDataType::USHORT,	GL_UNSIGNED_SHORT }
	};


	//////////////////////////////////////////////////////////////////////////


	// Returns the associated OpenGL system type based on the Bitmap's data type
	GLenum getGLType(BitmapDataType type)
	{
		auto it = openGLTypeMap.find(type);
		return it == openGLTypeMap.end() ? GL_INVALID_ENUM : it->second;
	}


	// the GL associated internal format associated with the BitmapColorType
	GLint getGLInternalFormat(BitmapColorType type, bool compressed /*= true*/)
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
	GLenum getGLFormat(BitmapColorType type)
	{
		auto it = openGLFormatMap.find(type);
		if (it == openGLFormatMap.end())
		{
			printMessage(MessageType::ERROR, "unable to find matching gl format for bitmap color of type: %d", static_cast<uint8_t>(type));
			return GL_INVALID_ENUM;
		}
		return it->second;
	}

	
	// Helper function to check bitmap validity
	bool checkBitmap(const BitmapBase& bitmap)
	{
		if (!bitmap.hasValidSettings())
		{
			printMessage(MessageType::ERROR, "unable to set texture from bitmap, invalid bitmap settings");
			return false;
		}

		if (!bitmap.hasData())
		{
			printMessage(MessageType::ERROR, "unable to set texture from bitmap, bitmap has no associated data");
			return false;
		}
		return true;
	}


	// Populates a Texture2D object with settings matching the bitmap
	bool getSettingsFromBitmap(const BitmapBase& bitmap, bool compress, Texture2DSettings& settings, nap::utility::ErrorState& errorState)
	{
		assert(checkBitmap(bitmap));

		// Fetch matching values
		GLint internal_format = getGLInternalFormat(bitmap.getColorType(), compress);
		if (!errorState.check(internal_format != GL_INVALID_VALUE, "Unable to determine internal format from bitmap"))
			return false;

		GLenum format = getGLFormat(bitmap.getColorType());
		if (!errorState.check(format != GL_INVALID_ENUM, "Unable to determine format from bitmap"))
			return false;

		GLenum type = getGLType(bitmap.getDataType());
		if (!errorState.check(type != GL_INVALID_ENUM, "Unable to determine texture type from bitmap"))
			return false;

		// Populate settings with fetched values from bitmap
		settings.internalFormat = internal_format;
		settings.format = format;
		settings.type = type;
		settings.width = bitmap.getWidth();
		settings.height = bitmap.getHeight();

		return true;
	}

	// Checks if the texture is compressed on the GPU and it's size on the GPU
	bool isCompressed(Texture& texture, GLint& size, GLint& type)
	{
		// Check compression
		GLint is_compressed(0);
		texture.bind();
		glGetTexLevelParameteriv(texture.getTargetType(), 0, GL_TEXTURE_COMPRESSED_ARB, &is_compressed);
		glGetTexLevelParameteriv(texture.getTargetType(), 0, GL_TEXTURE_INTERNAL_FORMAT, &type);
		glGetTexLevelParameteriv(texture.getTargetType(), 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size);
		texture.unbind();
		return is_compressed > 0;
	}


	const opengl::OpenGLTypeMap& getGLTypeMap()
	{
		return openGLTypeMap;
	}


	const opengl::OpenGLFormatMap& getGLFormatMap()
	{
		return openGLFormatMap;
	}

} // opengl
