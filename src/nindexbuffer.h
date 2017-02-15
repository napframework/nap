#pragma once

#include "nbuffer.h"

namespace opengl
{
	/**
	 * Settings associated with an indexed buffer
	 */
	struct IndexBufferSettings
	{
		IndexBufferSettings() = default;
		IndexBufferSettings(unsigned int indices, GLenum usage) : mCount(indices), mUsage(usage)	{ }

		unsigned int	mCount = 0;					// number of indices used to construct the triangles
		GLenum			mUsage = GL_STATIC_DRAW;	// defines the expected usage pattern of the data storage: https://www.opengl.org/sdk/docs/man4/html/glBufferData.xhtml

		/**
		 * @return total size of index buffer
		 */
		std::size_t getSize() const;

		/**
		 * @return if the settings associated with the index buffer
		 * are valid
		 */
		bool isValid() const;
	};


	/**
	 * Specialization of default buffer object, 
	 * the index buffer is used to specify the vertex connectivity, for more info:
	 * http://openglbook.com/chapter-3-index-buffer-objects-and-primitive-types.html
	 */
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(const IndexBufferSettings& settings);

		/**
		 * @return GL_ELEMENT_ARRAY_BUFFER
		 */
		virtual GLenum getBufferType() const override			{ return GL_ELEMENT_ARRAY_BUFFER; }

		/**
		 * @return Index settings associated with this buffer
		 */
		const IndexBufferSettings& getSettings() const			{ return mSettings; }

		/**
		 * @return the number of indices specified in this buffer
		 */
		std::size_t getCount() const							{ return mSettings.mCount; }

		/**
		 * Uploads index data to associated buffer
		 * Note that the pointer to that data needs to be of equal
		 * length as the current number of associated indices stored in the settings
		 * of this object
		 * @param indices: Index data to upload to the GPU
		 */
		void setData(unsigned int* indices);

		/**
		 * Uploads index data to associated buffer
		 * Note that data pointed to must mach the length specified by count
		 * @param indices: New set of indices that specify connectivity
		 * @param count: Total number of indices specified above
		 */
		void setData(unsigned int* indices, std::size_t count);

	protected:
		IndexBufferSettings mSettings;
	};
}
