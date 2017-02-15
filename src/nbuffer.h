#pragma once

// Local Includes
#include "nglutils.h"

// External Includes
#include <GL/glew.h>

namespace opengl
{
	/**
	 * Defines a buffer object on the GPU and acts as a base class for 
	 * all opengl derived buffer types. This object creation / destruction 
	 * as well as the internal buffer type. Note that this object does not
	 * own any of it's data, it only acts as an interface for buffers to the GPU
	 */
	class Buffer
	{
	public:
		/**
		 * Default constructor
		 */
		Buffer() = default;

		/**
		 * Default destructor
		 */
		virtual ~Buffer();

		// Don't allow copy, TODO: implement copy
		Buffer(const Buffer& other) = delete;
		Buffer& operator=(const Buffer& other) = delete;

		/**
		 * Override this method in a derived class to specify the buffer type
		 * for example: GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER etc
		 * @return this buffer's OpenGL type
		 */
		virtual GLenum getBufferType() const = 0;

		/**
		* Allocates the buffer on the GPU
		* Always call init after creation otherwise subsequent calls will fail
		*/
		void init();

		/**
		* @return if the buffer is allocated on the GPU
		*/
		bool isAllocated() const						{ return mId != 0; }

		/**
		* @return the id associated with this buffer on the GPU
		*/
		GLuint getId() const							{ return mId; }

		/**
		* binds this vertex buffer on the GPU for subsequent GPU calls
		*/
		bool bind();

		/**
		* unbinds this vertex buffer on the GPU for subsequent GPU calls
		*/
		bool unbind();

	private:
		// Buffer GPU ID
		GLuint					mId = 0;
	};

} // opengl
