#pragma once

#include <GL/glew.h>
#include <vector>

namespace opengl
{
	/**
	 * TextureParameters
	 *
	 * Universal Texture parameters that can be applied to all texture types
	 */
	struct TextureParameters final
	{
		GLint minFilter =		GL_LINEAR_MIPMAP_LINEAR;		//< Filter used when sampling down
		GLint maxFilter =		GL_LINEAR;						//< Filter used when sampling up
		GLint wrapVertical =	GL_CLAMP_TO_EDGE;				//< Method used for clamping texture vertically
		GLint wrapHorizontal =	GL_CLAMP_TO_EDGE;				//< Method used for clamping texture horizontally
		GLint maxLodLevel =		20;								// Maximum number of lods
	};


	/**
	 * Base OpengGL Texture
	 *
	 * Acts as an abstract class for any type of texture
	 * Manages GPU texture resource
	 */
	class BaseTexture
	{
	public:
		/**
		 * Texture
		 *
		 * Creates / Destroys associated OpenGL texture
		 */
		BaseTexture(GLenum inTargetType);
		virtual ~BaseTexture();

		/**
		 * Copy is not allowed
		 *
		 * We can't copy a hardware texture from GPU space a to b
		 * At least for now
		 */
		BaseTexture(const BaseTexture& other) = delete;
		BaseTexture& operator=(const BaseTexture& other) = delete;

		/**
		 * init call uploads current parameters to GPU and creates a texture
		 * make sure to call this function after creating the object!
		 */
		void init(const TextureParameters& parameters);

		/**
		 * All future texture functions will modify this texture
		 * Derived classes need to implement this for 2d or 3d textures
		 */
		void bind();
		
		/**
		 * Unbind texture
		 */
		void unbind();

		/**
		 * @Return current GPU texture location
		 */
		GLuint getTextureId() const						{ return mTextureId; }

		/**
		 * @return currently used texture parameters
		 */
		const TextureParameters& getParameters() const		{ return mParameters; }

		/**
		* Creates mip-maps for the texture on the GPU
		* This call will only work if data is available to the texture on the GPU 
		* and the min min filter is of a MipMap enabled type. To check if a texture supports mip-mapping use
		* opengl::isMipMap(x)
		*/
		void generateMipMaps();

		GLenum getTargetType() const { return mTargetType; }

	private:
		bool isAllocated() const { return mTextureId != -1; }

		void setParameters(const TextureParameters& settings);

	protected:
		// Texture ID
		GLuint				mTextureId;				// Currently associated texture ID on GPU
		GLenum				mTargetType;
		TextureParameters	mParameters;			// Texture specific 
	};


	//////////////////////////////////////////////////////////////////////////

	/**
	* Texture2Dsettings
	*
	* Data associated with a 2d texture
	*/
	struct Texture2DSettings
	{
	public:
		GLint internalFormat = GL_RGB;		//< Specifies the number of color components in the texture
		GLsizei width = 0;					//< Specifies the width of the texture
		GLsizei height = 0;					//< Specifies the height of the texture
		GLenum format = GL_BGR;				//< Specifies the format of the pixel data
		GLenum type = GL_UNSIGNED_BYTE;		//< Data type of the pixel data (GL_UNSIGNED_BYTE etc..)
	};

	/**
	 * Texture2D
	 *
	 * Represents a 2 dimensional texture on the GPU
	 */
	class Texture2D : public BaseTexture
	{
	public:
		// Default constructor
		Texture2D();

		void init(const Texture2DSettings& textureSettings, const TextureParameters& parameters);

		const Texture2DSettings& getSettings() const { return mSettings; }

		/**
		 * setData
		 * 
		 * Uploads 2d pixel data to the GPU
		 * Make sure that the data the pointer points at matches the size of the texture settings provided!
		 */
		void setData(void* data);

		int getDataSize() const;
		void getData(std::vector<uint8_t>& data);

	private:
		Texture2DSettings mSettings;
	};
} // opengl
