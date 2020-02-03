// Local Includes
#include "nbuffer.h"

// External Includes
#include "vulkan/vulkan.h"
#include <assert.h>
#include <string.h>

namespace opengl
{
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		return -1;
	}


	// Uploads the data block to the GPU
	void Buffer::setDataInternal(VkPhysicalDevice physicalDevice, VkDevice device, void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage)
	{
		// TODO: handle 'usage': dynamic/state
		// TODO: handle reallocation of buffer
		// TODO: handle growing of buffer
		// TODO: destroy buffers


		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = elementSize * numVertices;
		bufferInfo.usage = usage;// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS)
		{
			// TODO: error handling
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &mMemory) != VK_SUCCESS)
		{
			// TODO: error handling
			return;
		}

		vkBindBufferMemory(device, mBuffer, mMemory, 0);

		void* mapped_memory;
		vkMapMemory(device, mMemory, 0, bufferInfo.size, 0, &mapped_memory);
		memcpy(mapped_memory, data, (size_t)bufferInfo.size);
		vkUnmapMemory(device, mMemory);
		/*
		assert(reservedNumVertices >= numVertices);
		bind();

		mCurSize = getGLTypeSize(mType) * mNumComponents * numVertices;
		if (mCurSize > mCurCapacity)
		{
		mCurCapacity = getGLTypeSize(mType) * mNumComponents * reservedNumVertices;
		glBufferData(getBufferType(), mCurCapacity, nullptr, mUsage);
		}

		glBufferSubData(getBufferType(), 0, mCurSize, data);
		glAssert();
		unbind();
		*/
	}

} // opengl