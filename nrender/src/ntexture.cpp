// Local Includes
#include "ntexture.h"
#include "nglutils.h"

// External Includes
#include <iostream>
#include <unordered_set>
#include <assert.h>

namespace opengl
{
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
} // opengl