// Local Includes
#include "ntexture2d.h"
#include "nglutils.h"
#include <assert.h>

namespace opengl
{
	// Returns number of components each texel has in this format
	static int getNumComponents(GLenum format)
	{
		switch (format)
		{
			case GL_STENCIL_INDEX: 
			case GL_DEPTH_COMPONENT: 
			case GL_RED: 
			case GL_GREEN: 
			case GL_BLUE: 
			case GL_RED_INTEGER:
			case GL_GREEN_INTEGER:
			case GL_BLUE_INTEGER:
			case GL_DEPTH_STENCIL:
				return 1;

			case GL_RG: 
			case GL_RG_INTEGER:
				return 2;

			case GL_RGB: 			
			case GL_BGR: 
			case GL_RGB_INTEGER:
			case GL_BGR_INTEGER:
				return 3;

			case GL_BGRA: 
			case GL_RGBA:
			case GL_RGBA_INTEGER:
			case GL_BGRA_INTEGER:
				return 4;
		}

		assert(false);
		return -1;
	}

	// Returns What the size in bytes is of a component type
	static int getComponentSize(GLenum type)
	{
		switch (type)
		{
			case GL_UNSIGNED_BYTE:
			case GL_BYTE: 
			case GL_UNSIGNED_BYTE_3_3_2:
			case GL_UNSIGNED_BYTE_2_3_3_REV:
				return 1;

			case GL_UNSIGNED_SHORT: 
			case GL_SHORT: 
			case GL_HALF_FLOAT:
			case GL_UNSIGNED_SHORT_5_6_5:
			case GL_UNSIGNED_SHORT_5_6_5_REV:
			case GL_UNSIGNED_SHORT_4_4_4_4:
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
				return 2;

			case GL_UNSIGNED_INT: 
			case GL_UNSIGNED_INT_8_8_8_8:
			case GL_UNSIGNED_INT_8_8_8_8_REV:
			case GL_UNSIGNED_INT_10_10_10_2:
			case GL_UNSIGNED_INT_2_10_10_10_REV:
			case GL_UNSIGNED_INT_24_8:
			case GL_UNSIGNED_INT_10F_11F_11F_REV:
			case GL_UNSIGNED_INT_5_9_9_9_REV:
			case GL_INT: 
			case GL_FLOAT:
			case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
				return 4;				
		}

		assert(false);
		return -1;
	}

	Texture2D::Texture2D() :
		Texture(GL_TEXTURE_2D)
	{
	}


	void Texture2D::init(const Texture2DSettings& settings, const TextureParameters& parameters) 
	{
		mSettings = settings;
		Texture::init(parameters);
		setData(nullptr);
	}


	/**
	 * Uploads the 2D texture data to the GPU
	 */
	void Texture2D::setData(void* data)
	{
		bind();

		// Upload texture data
		glTexImage2D(GL_TEXTURE_2D,
			0,
			mSettings.internalFormat,
			mSettings.width,
			mSettings.height,
			0,
			mSettings.format,
			mSettings.type ,
			data);
		glAssert();

		unbind();

		// Auto generate mip maps if data is valid and we're dealing with a mip mapable type
		if(isMipMap(mParameters.minFilter) && data != nullptr)
			generateMipMaps();
	}


	int Texture2D::getDataSize() const
	{
		return getNumComponents(mSettings.format) * getComponentSize(mSettings.type) * mSettings.width * mSettings.height;
	}


	void Texture2D::getData(std::vector<uint8_t>& data)
	{
		data.resize(getDataSize());

		bind();
		glGetTexImage(GL_TEXTURE_2D, 0, mSettings.format, mSettings.type, data.data());
		unbind();
	}

} // opengl