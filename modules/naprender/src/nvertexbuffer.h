#pragma once

// Local Includes
#include "nbuffer.h"
#include "vulkan/vulkan_core.h"

// External Includes
//#include <GL/glew.h>
#include <stdint.h>

namespace opengl
{
	/**
	 * Defines a vertex buffer on the GPU that is associated with a single set of vertex data
	 * Vertex data is arbitrary vertex data such as position, uv, color etc.
	 * This object does not manage or owns any data
	 */
	class NAPAPI VertexAttributeBuffer : public Buffer
	{
	public:
		VertexAttributeBuffer() = default;

		VertexAttributeBuffer(VkFormat inFormat);

		VkFormat getFormat() const { return mFormat; }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function binds the buffer before uploading the data and
		 * automatically allocates GPU memory if required. 
		 * Ensure reservedNumVertices >= numVertices. Capacity is calculated based on reservedNumVertices.
		 * @param data pointer to the block of data that needs to be uploaded
		 * @param numVertices number of vertices represented by data
		 * @param reservedNumVertices used when current capacity is lower than current size. Used to calculate GPU buffer size.
		 */
		void setData(VkPhysicalDevice physicalDevice, VkDevice device, void* data, size_t numVertices, size_t reservedNumVertices);

	private:
		VkFormat		mFormat;
		int				mVertexSize			= -1;
	};

	int getVertexSize(VkFormat format);
}
