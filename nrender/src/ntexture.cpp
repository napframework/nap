// Local Includes
#include "ntexture.h"
#include "nglutils.h"

// External Includes
#include <iostream>
#include <unordered_set>
#include <assert.h>

namespace opengl
{
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


	// Generates a texture on the GPU side
	BaseTexture::BaseTexture(GLenum inTargetType) :
		mTargetType(inTargetType),
		mTextureId(-1)
	{}


	// Remove texture
	BaseTexture::~BaseTexture()
	{
		if (isAllocated())
			glDeleteTextures(1, &mTextureId);
	}


	//  uploads current parameters to GPU
	void BaseTexture::init(const TextureParameters& parameters)
	{
		assert(!isAllocated());

		// Generate textures
		glGenTextures(1, &mTextureId);
		glAssert();

		// Upload texture parameters
		mParameters = parameters;
		setParameters(parameters);
	}

	// Binds the texture
	void BaseTexture::bind()
	{
		assert(isAllocated());
		glBindTexture(mTargetType, mTextureId);
	}


	// Detach the texture from the current Texture Unit
	void BaseTexture::unbind()
	{
		assert(isAllocated());
		glBindTexture(mTargetType, 0);
	}


	// Updates settings associated with this GPU texture
	void BaseTexture::setParameters(const TextureParameters& parameters)
	{
		bind();

		glTexParameteri(mTargetType, GL_TEXTURE_MIN_FILTER, parameters.minFilter);
		glTexParameteri(mTargetType, GL_TEXTURE_MAG_FILTER, parameters.maxFilter);
		glTexParameteri(mTargetType, GL_TEXTURE_WRAP_S,		parameters.wrapHorizontal);
		glTexParameteri(mTargetType, GL_TEXTURE_WRAP_T,		parameters.wrapVertical);
		glTexParameteri(mTargetType, GL_TEXTURE_MAX_LEVEL,  parameters.maxLodLevel);

		// Store settings
		mParameters = parameters;

		// Unbind texture
		unbind();
	}


	// Creates mip maps for the texture on the GPU
	void BaseTexture::generateMipMaps()
	{
		bind();

		// Generate and check for errors
		glGenerateMipmap(mTargetType);
		glAssert();

		unbind();
	}

	/////////////////////////////////////////////////////////////////////////

	Texture2D::Texture2D() :
		BaseTexture(GL_TEXTURE_2D)
	{
	}

	void Texture2D::init(const Texture2DSettings& settings, const TextureParameters& parameters) 
	{
		mSettings = settings;
		BaseTexture::init(parameters);
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