// Local Includes
#include "ntexture.h"
#include "nglutils.h"

// External Includes
#include <iostream>
#include <unordered_set>

namespace opengl
{
	// Generates a texture on the GPU side
	BaseTexture::BaseTexture()
	{}


	// Remove texture
	BaseTexture::~BaseTexture()
	{
		if (isAllocated())
			glDeleteTextures(1, &mTextureId);
	}


	//  uploads current parameters to GPU
	void BaseTexture::init()
	{
		if (isAllocated())
		{
			printMessage(MessageType::WARNING, "texture already allocated");
			printMessage(MessageType::WARNING, "old texture is invalidated");
			glDeleteTextures(1, &mTextureId);
		}

		// Generate textures
		glGenTextures(1, &mTextureId);

		// Upload texture parameters
		updateParameters(mParameters);
	}


	// Binds the texture
	bool BaseTexture::bind() const
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind texture : texture is not allocated");
			return false;
		}
		glBindTexture(getTargetType(), mTextureId);
		return true;
	}


	// Detach the texture from the current Texture Unit
	bool BaseTexture::unbind() const
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to unbind texture : texture is not allocated");
			return false;
		}
		glBindTexture(getTargetType(), 0);
		return true;
	}


	// Updates settings associated with this GPU texture
	void BaseTexture::updateParameters(const TextureParameters& settings)
	{
		if (!bind())
			return;

		glTexParameteri(getTargetType(), GL_TEXTURE_MIN_FILTER, settings.minFilter);
		glTexParameteri(getTargetType(), GL_TEXTURE_MAG_FILTER, settings.maxFilter);
		glTexParameteri(getTargetType(), GL_TEXTURE_WRAP_S,		settings.wrapHorizontal);
		glTexParameteri(getTargetType(), GL_TEXTURE_WRAP_T,		settings.wrapVertical);

		// Store settings
		mParameters = settings;

		// Unbind texture
		unbind();
	}


	/**
	 * setData
	 * 
	 * Binds the current texture to be the active texture object
	 * Calls onSetData to actually upload data to GPU
	 */
	void BaseTexture::setData(void* data)
	{
		if (!bind())
			return;
		
		// Upload texture specific data and generate mip maps
		onSetData(data);
		
		// Unbind
		unbind();

		// Auto generate mip maps if data is valid and we're dealing with a mip mapable type
		if(isMipMap(mParameters.minFilter) && data != nullptr)
			generateMipMaps();
	}


	// Creates mip maps for the texture on the GPU
	void BaseTexture::generateMipMaps()
	{
		if (!bind())
			return;

		// Generate and check for errors
		glGenerateMipmap(getTargetType());
		
		// Check if successful
		GLenum error_code = glGetError();
		if (error_code != GL_NO_ERROR)
			printMessage(MessageType::ERROR, "unable to generate mip-map for texture, unknown error");

		unbind();
	}

	/////////////////////////////////////////////////////////////////////////

	/**
	 * Uploads the 2D texture data to the GPU
	 */
	void Texture2D::onSetData(void* data)
	{
		// Upload texture data
		glTexImage2D(getTargetType(),
			mSettings.level,
			mSettings.internalFormat,
			mSettings.width,
			mSettings.height,
			0,
			mSettings.format,
			mSettings.type ,
			data);
	}
} // opengl