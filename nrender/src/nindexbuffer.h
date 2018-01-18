#pragma once

#include "nbuffer.h"
#include <vector>

namespace opengl
{

	/**
	 * Specialization of default buffer object, 
	 * the index buffer is used to specify the vertex connectivity, for more info:
	 * http://openglbook.com/chapter-3-index-buffer-objects-and-primitive-types.html
	 */
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(GLenum usage);

		/**
		 * @return GL_ELEMENT_ARRAY_BUFFER
		 */
		virtual GLenum getBufferType() const override			{ return GL_ELEMENT_ARRAY_BUFFER; }

		/**
		 * @return GL_UNSIGNED_INT
		 */
		virtual GLenum getType() const							{ return GL_UNSIGNED_INT; }

		/**
		 * @return the number of indices specified in this buffer
		 */
		std::size_t getCount() const							{ return mCount; }

		/**
		 * Uploads index data to associated buffer
		 * Note that the pointer to that data needs to be of equal
		 * length as the current number of associated indices stored in the settings
		 * of this object
		 * @param indices: Index data to upload to the GPU
		 */
		void setData(const std::vector<unsigned int>& indices);

	private:
		size_t		mCurCapacity = 0;			// Amount of memory reserved
		size_t		mCount = 0;					// number of indices used to construct the triangles
		GLenum		mUsage = GL_STATIC_DRAW;	// defines the expected usage pattern of the data storage: https://www.opengl.org/sdk/docs/man4/html/glBufferData.xhtml
	};
}
