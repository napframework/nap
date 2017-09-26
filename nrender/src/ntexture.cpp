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
	Texture::Texture(GLenum inTargetType) :
		mTargetType(inTargetType),
		mTextureId(-1)
	{}


	// Remove texture
	Texture::~Texture()
	{
		if (isAllocated())
			glDeleteTextures(1, &mTextureId);
	}


	//  uploads current parameters to GPU
	void Texture::init(const TextureParameters& parameters)
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
	void Texture::bind()
	{
		assert(isAllocated());
		glBindTexture(mTargetType, mTextureId);
	}


	// Detach the texture from the current Texture Unit
	void Texture::unbind()
	{
		assert(isAllocated());
		glBindTexture(mTargetType, 0);
	}


	// Updates settings associated with this GPU texture
	void Texture::setParameters(const TextureParameters& parameters)
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
	void Texture::generateMipMaps()
	{
		bind();

		// Generate and check for errors
		glGenerateMipmap(mTargetType);
		glAssert();

		unbind();
	}

} // opengl