#pragma once

// Local Includes
#include "ngltypes.h"
#include "nglutils.h"
#include "nbuffer.h"

// External Includes
#include <GL/glew.h>
#include <stdint.h>

namespace opengl
{
	/**
	 * Defines a vertex buffer on the GPU that is associated with a single set of vertex data
	 * Vertex data is arbitrary vertex data such as position, uv, color etc.
	 * This object does not manage or owns any data
	 */
	class VertexAttributeBuffer : public Buffer
	{
	public:
		VertexAttributeBuffer() = default;

		VertexAttributeBuffer(GLenum type, unsigned int numComponents, GLenum usage);

		/**
		 * @return the buffer OpenGL type, INVALID_ENUM if not specified
		 */
		 virtual GLenum getType() const							{ return mType; }

		 /**
		  * @return the buffer opengl type
		  */
		 virtual GLenum getBufferType() const override			{ return GL_ARRAY_BUFFER; }

		/**
		 * @return the number of components associated with the buffer
		 */
		unsigned int getNumComponents() const					{ return mNumComponents; }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function binds the buffer before uploading the data and
		 * automatically allocates GPU memory if required. 
		 * Ensure reservedNumVertices >= numVertices. Capacity is calculated based on reservedNumVertices.
		 * @param data pointer to the block of data that needs to be uploaded
		 * @param numVertices number of vertices represented by data
		 * @param reservedNumVertices used when current capacity is lower than current size. Used to calculate GPU buffer size.
		 */
		void setData(void* data, size_t numVertices, size_t reservedNumVertices);

	private:
		GLenum			mType				= GL_INVALID_ENUM;	// defines the internal GL type of the buffer
		unsigned int	mNumComponents		= 0;				// defines the number of components (3 for color, 2 for uv's etc)
		size_t			mCurCapacity		= 0;				// Amount of memory reserved
		size_t			mCurSize			= 0;				// defines the number of points in the buffer
		GLenum			mUsage				= GL_STATIC_DRAW;	// defines the expected usage pattern of the data storage: https://www.opengl.org/sdk/docs/man4/html/glBufferData.xhtml
	};
}
