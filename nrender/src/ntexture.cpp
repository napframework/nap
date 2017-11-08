// Local Includes
#include "ntexture.h"
#include "nglutils.h"
#include "rtti/typeinfo.h"

// External Includes
#include <iostream>
#include <unordered_set>
#include <assert.h>

RTTI_BEGIN_ENUM(opengl::ETextureUsage)
	RTTI_ENUM_VALUE(opengl::ETextureUsage::Static,			"Static"),
	RTTI_ENUM_VALUE(opengl::ETextureUsage::DynamicRead,		"DynamicRead"),
	RTTI_ENUM_VALUE(opengl::ETextureUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

namespace opengl
{

	// Generates a texture on the GPU side
	Texture::Texture(GLenum inTargetType) :
		mTextureId(-1),
        mTargetType(inTargetType)
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


	void Texture::initPBO(GLuint& pbo, ETextureUsage usage, int textureSizeInBytes)
	{
		if (usage == ETextureUsage::DynamicWrite)
		{
			glGenBuffers(1, &pbo);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, textureSizeInBytes, NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
		else if (usage == ETextureUsage::DynamicRead)
		{
			glGenBuffers(1, &pbo);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
			glBufferData(GL_PIXEL_PACK_BUFFER, textureSizeInBytes, NULL, GL_DYNAMIC_READ);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}
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
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

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
