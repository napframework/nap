#pragma once

#include <GL/glew.h>

namespace opengl
{
	/**
	 * TextureParameters
	 *
	 * Universal Texture parameters that can be applied to all texture types
	 */
	struct TextureParameters
	{
		TextureParameters()  =	default;
		~TextureParameters() =	default;

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
		BaseTexture();
		virtual ~BaseTexture();

		/**
		 * Copy is not allowed
		 *
		 * We can't copy a hardware texture from GPU space a to b
		 * At least for now
		 */
		 // Copy is allowed
		BaseTexture(const BaseTexture& other) = delete;
		BaseTexture& operator=(const BaseTexture& other) = delete;

		/**
		 * init call uploads current parameters to GPU and creates a texture
		 * make sure to call this function after creating the object!
		 */
		void init();

		/**
		 * sets the parameters to be used when initializing the Texture
		 * @param parameters the parameters to associate with this texture
		 */
		void setParameters(const opengl::TextureParameters& parameters);

		/**
		 * All future texture functions will modify this texture
		 * Derived classes need to implement this for 2d or 3d textures
		 */
		bool bind();
		
		/**
		 * Unbind texture
		 */
		bool unbind();

		/**
		 * @return texture target type (2D, 3D, 2D array etc.)
		 */
		virtual GLenum getTargetType() const = 0;

		/**
		 * @Return current GPU texture location
		 */
		GLuint getTextureId() const						{ return mTextureId; }

		/**
		 * Updates generic texture settings, pushes changes immediately to GPU
		 * Note that when working with a mipmapped texture automatic mipmap generation is enabled
		 * 
		 */
		void updateParameters(const TextureParameters& settings);

		/**
		 * @return currently used texture parameters
		 */
		const TextureParameters& getParameters() const		{ return mParameters; }

		/**
		 * @return if the texture object is generated and allocated on the GPU
		 * The texture is allocated the moment init() is called
		 */
		bool isAllocated() const							{ return mTextureId != 0; }

		/**
		 * Uploads the data on the GPU for this texture object
		 * This call is forwarded to onSetData after successfully binding the texture object
		 * If the current parameters contain a mip map filter mip maps will be generated
		 * @param pointer to the image data in memory, can be a nullptr if data is set externally using an FBO
		 */
		void setData(void* data);

		/**
		* Creates mip-maps for the texture on the GPU
		* This call will only work if data is available to the texture on the GPU 
		* and the min min filter is of a MipMap enabled type. To check if a texture supports mip-mapping use
		* opengl::isMipMap(x)
		*/
		void generateMipMaps();

	protected:
		// Texture ID
		GLuint				mTextureId = 0;			// Currently associated texture ID on GPU
		TextureParameters	mParameters;			// Texture specific 

		/**
		 * onSetData
		 * 
		 * called when data has to be uploaded to the GPU
		 * uses the texture settings object as reference
		 */
		virtual void onSetData(void* data) = 0;
	};


	//////////////////////////////////////////////////////////////////////////


	/**
	 * Texture
	 *
	 * Defines a texture associated with a set of settings
	 * Settings are used to upload data to the GPU for the specified target type
	 * Using this interface settings are handled automatically before data is set
	 */
	template<typename S>
	class Texture : public BaseTexture
	{
	public:
		Texture() = default;

		/**
		 * getSettings / setSettings
		 *
		 * gets or sets texture specific texture settings
		 * it's important to set settings before setting data
		 * the settings are used to push data on to the GPU
		 */
		const S&	getSettings()						{ return mSettings; }
		void		setSettings(const S& settings)		{ mSettings = settings; }

		/**
		 * setData
		 *
		 * Sets the data based on the settings provided
		 * Note that settings need to match data size of data pointer!
		 */
		virtual void setData(const S& settings, void* data)
		{
			mSettings = settings;
			BaseTexture::setData(data);
		}

		/**
		 * Specifies a texture in GPU memory based on the provided settings
		 * This call is similar to setData but without actually uploading any data from memory
		 * Use this when data is provided externally, for example a framebuffer
		 */
		void allocate(const S& settings)
		{
			setData(settings, nullptr);
		}

	protected:
		S mSettings;	//< Texture associated settings
	};


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
	class Texture2D : public Texture<Texture2DSettings>
	{
	public:
		// Default constructor
		Texture2D() = default;

		/**
		 * getTargetType
		 * 
		 * This texture's OpenGL target type is a GL_TEXTURE_2D
		 */
		GLenum getTargetType() const override		{ return GL_TEXTURE_2D; }

		/**
		 * setData
		 * 
		 * Uploads 2d pixel data to the GPU
		 * Make sure that the data the pointer points at matches the size of the texture settings provided!
		 */
		void onSetData(void* data) override;
	};
} // opengl
