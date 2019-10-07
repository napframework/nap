// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include "vulkan/vulkan.h"
#include <assert.h>

namespace opengl
{


	int getVertexSize(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_R8_SINT:
				return 1;

			case VK_FORMAT_R32_SFLOAT:
			case VK_FORMAT_R32_SINT:
				return 4;

			case VK_FORMAT_R64_SFLOAT:
			case VK_FORMAT_R32G32_SFLOAT:
				return 8;

			case VK_FORMAT_R32G32B32_SFLOAT:
				return 12;

			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return 16;

			default:
				assert(false);
		}

		return -1;
	}

	VertexAttributeBuffer::VertexAttributeBuffer(VkFormat inFormat) :
		mFormat(inFormat),
		mVertexSize(getVertexSize(inFormat))
	{
	}


	// Uploads the data block to the GPU
	void VertexAttributeBuffer::setData(VkPhysicalDevice physicalDevice, VkDevice device, void* data, size_t numVertices, size_t reservedNumVertices)
	{
		setDataInternal(physicalDevice, device, data, mVertexSize, numVertices, reservedNumVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

} // opengl