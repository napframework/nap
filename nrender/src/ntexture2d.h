#pragma once

#include "ntexture.h"

namespace opengl
{
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
	class Texture2D : public Texture
	{
	public:
		// Default constructor
		Texture2D();

		void init(const Texture2DSettings& textureSettings, const TextureParameters& parameters);

		/**
		 * @return Texture2D settings object
		 */
		const Texture2DSettings& getSettings() const { return mSettings; }

		/**
		 * setData
		 * 
		 * Uploads 2d pixel data to the GPU
		 * Make sure that the data the pointer points at matches the size of the texture settings provided!
		 */
		void setData(void* data);

		/**
		 * @return The size of the texture when copied to/from CPU.
		 */
		int getDataSize() const;

		/**
		 * @return Texture2D settings object
		 */
		void getData(std::vector<uint8_t>& data);

	private:
		Texture2DSettings mSettings;		///< Settings object
	};
} // opengl
