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
		GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;		//< Filter used when sampling down
		GLint maxFilter = GL_LINEAR;						//< Filter used when sampling up
		GLint wrapVertical = GL_CLAMP_TO_EDGE;				//< Method used for clamping texture vertically
		GLint wrapHorizontal = GL_CLAMP_TO_EDGE;				//< Method used for clamping texture horizontally
		GLint maxLodLevel = 20;								// Maximum number of lods
	};


	/**
	 * Base OpengGL Texture
	 *
	 * Acts as an abstract class for any type of texture
	 * Manages GPU texture resource
	 */
	class Texture
	{
	public:
		/**
		 * Texture
		 *
		 * Creates / Destroys associated OpenGL texture
		 */
		Texture(GLenum inTargetType);
		virtual ~Texture();

		/**
		 * Copy is not allowed
		 *
		 * We can't copy a hardware texture from GPU space a to b
		 * At least for now
		 */
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

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
		GLuint getTextureId() const { return mTextureId; }

		/**
		 * @return currently used texture parameters
		 */
		const TextureParameters& getParameters() const { return mParameters; }

		/**
		* Creates mip-maps for the texture on the GPU
		* This call will only work if data is available to the texture on the GPU
		* and the min min filter is of a MipMap enabled type. To check if a texture supports mip-mapping use
		* opengl::isMipMap(x)
		*/
		void generateMipMaps();

		/**
		 * @return opengl texture type
		 */
		GLenum getTargetType() const { return mTargetType; }

	private:
		/**
		 * @return wheter texture is allocated (which happens at init() time)
		 */
		bool isAllocated() const { return mTextureId != -1; }

		/**
		 * Updates texture parameters
		 */
		void setParameters(const TextureParameters& settings);

	protected:
		// Texture ID
		GLuint				mTextureId;				// Currently associated texture ID on GPU
		GLenum				mTargetType;			// Opengl type of texture
		TextureParameters	mParameters;			// Texture specific 
	};
}