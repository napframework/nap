// Local Includes
#include "descriptorsetcache.h"
#include "descriptorsetallocator.h"
#include "renderservice.h"
#include "uniformdeclarations.h"
#include "materialcommon.h"

namespace nap
{
	bool createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& bufferAllocation, VmaAllocationInfo& bufferAllocationInfo)
	{
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		return vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &bufferAllocation, &bufferAllocationInfo) == VK_SUCCESS;
	}


	//////////////////////////////////////////////////////////////////////////

	DescriptorSetCache::DescriptorSetCache(RenderService& renderService, VkDescriptorSetLayout layout, DescriptorSetAllocator& descriptorSetAllocator) :
		mRenderService(&renderService),
		mDescriptorSetAllocator(&descriptorSetAllocator),
        mLayout(layout)
	{
		mUsedList.resize(mRenderService->getMaxFramesInFlight());
	}

	
	DescriptorSetCache::~DescriptorSetCache()
	{
		// We assume the cache is destroyed after all rendering has completed, so it is safe to assume we can move any 'used' sets to the freelist
		for (int frame = 0; frame < mUsedList.size(); ++frame)
			release(frame);

		// Now free all buffers for the allocated sets
		for (DescriptorSet& descriptor_set : mFreeList)
		{
			for (DescriptorSetBuffer& buffer : descriptor_set.mBuffers)
				vmaDestroyBuffer(mRenderService->getVulkanAllocator(), buffer.mBuffer, buffer.mAllocation);
		}
	}


	const DescriptorSet& DescriptorSetCache::acquire(const std::vector<UniformBufferObject>& uniformBufferObjects, int numSamplers)
	{
		int frame_index = mRenderService->getCurrentFrameIndex();
		DescriptorSetList& used_list = mUsedList[frame_index];
		
		// If there are available DescriptorSets, we can use them directly
		if (!mFreeList.empty())
		{
			// Move the last item to the used list for this frame (= acquire this free item)
			used_list.splice(used_list.end(), mFreeList, --mFreeList.end());
			return used_list.back();
		}

		// No free items, let's allocate one from the pool. This will allocate a DescriptorSet from a pool that is *compatible* with our layout.
		// Compatible means that it has the same amount of uniform buffers and same amount of samplers.
		DescriptorSet descriptor_set;
		descriptor_set.mLayout = mLayout;
		descriptor_set.mSet = mDescriptorSetAllocator->allocate(mLayout, uniformBufferObjects.size(), numSamplers);

		int num_descriptors = uniformBufferObjects.size();
		std::vector<VkWriteDescriptorSet> ubo_descriptors;
		ubo_descriptors.resize(num_descriptors);

		std::vector<VkDescriptorBufferInfo> descriptor_buffers(num_descriptors);
		descriptor_buffers.resize(num_descriptors);

		// Now we allocate the UBO buffers from the global Vulkan allocator, and bind the buffers to our DescriptorSet
		for (int ubo_index = 0; ubo_index < uniformBufferObjects.size(); ++ubo_index)
		{
			const UniformBufferObject& ubo = uniformBufferObjects[ubo_index];
			const UniformBufferObjectDeclaration& ubo_declaration = *ubo.mDeclaration;

			DescriptorSetBuffer buffer;
			utility::ErrorState error_state;
			bool success = createBuffer(mRenderService->getVulkanAllocator(), ubo_declaration.mSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, buffer.mBuffer, buffer.mAllocation, buffer.mAllocationInfo);
			assert(success);

			descriptor_set.mBuffers.push_back(buffer);

			VkDescriptorBufferInfo& bufferInfo = descriptor_buffers[ubo_index];
			bufferInfo.buffer = buffer.mBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet& ubo_descriptor = ubo_descriptors[ubo_index];
			ubo_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ubo_descriptor.dstSet = descriptor_set.mSet;
			ubo_descriptor.dstBinding = ubo_declaration.mBinding;
			ubo_descriptor.dstArrayElement = 0;
			ubo_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ubo_descriptor.descriptorCount = 1;
			ubo_descriptor.pBufferInfo = &bufferInfo;
		}

		vkUpdateDescriptorSets(mRenderService->getDevice(), ubo_descriptors.size(), ubo_descriptors.data(), 0, nullptr);

		used_list.emplace_back(std::move(descriptor_set));
		return used_list.back();
	}

	void DescriptorSetCache::release(int frameIndex)
	{
		DescriptorSetList& used_list = mUsedList[frameIndex];
		mFreeList.splice(mFreeList.end(), used_list);
	}

}

