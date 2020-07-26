#pragma once

// Internal includes
#include "vk_mem_alloc.h"

// External Includes
#include <vector>
#include <list>
#include <array>
#include <utility/dllexport.h>
#include <vulkan/vulkan_core.h>

namespace nap
{
	// Forward Declares
	class UniformBufferObject;
	class SamplerInstance;
	class RenderService;
	class DescriptorSetAllocator;

	/** 
	 * Wrapper around VkBuffer that holds allocation information.
	 */
	struct DescriptorSetBuffer
	{
		VkBuffer			mBuffer = VK_NULL_HANDLE;
		VmaAllocation		mAllocation = VK_NULL_HANDLE;
		VmaAllocationInfo	mAllocationInfo;
	};

	/** 
	 * Wrapper around VkDescriptorSet that also contains the allocated buffers for a DescriptorSet.
	 */
	struct DescriptorSet
	{
		VkDescriptorSetLayout				mLayout;
		VkDescriptorSet						mSet;
		std::vector<DescriptorSetBuffer>	mBuffers;
	};

	/** 
	 * Responsible for caching DescriptorSets and allocating them when the DescriptorSet is not in the cache.
	 * A DescriptorSetAllocator allocates DescriptorSets for a specific VkDescriptorSetLayout. Any DescriptorSet
	 * that is returned will be compatible with that VkDescriptorSetLayout.
	 *
	 * Internally it will maintain a free-list of available DescriptorSets. When a VkDescriptorSet is acquired,
	 * it is marked for use by that frame (the current RenderService frame is used). When a frame is fully  
	 * completed, release should be called for that frame so that the resources are return to the freel-ist, to 
	 * be used by subsequent frames.
	 */
	class NAPAPI DescriptorSetCache final
	{
	public:
		DescriptorSetCache(RenderService& renderService, VkDescriptorSetLayout layout, DescriptorSetAllocator& descriptorSetAllocator);
		~DescriptorSetCache();

		/**
		 * Acquires a DescriptorSet from the cache (or allocated it if not in the cache). For new DescriptorSets,
		 * also allocates Buffers for each UBO from the global Vulkan allocator. The result is a DescriptorSet
		 * that is fully compatible with the DescriptorSetLayout.
		 * @param uniformBufferObjects The list of UBOs for this DescriptorSet.
		 * @param samplers The list of samplers for this DescriptorSet
		 * @return A DescriptorSet that is compatible with the VkDescriptorLayout that was passed upon creation.
		 */
		const DescriptorSet& acquire(const std::vector<UniformBufferObject>& uniformBufferObjects, int numSamplers);

		/**
		 * Releases all DescriptorSets to the internal pool for use by other frames. 
		 * @param frameIndex The frame index that was completed by the render system and for which no resources are in use anymore.
		 */
		void release(int frameIndex);

	private:
		using DescriptorSetList = std::list<DescriptorSet>;
		using DescriptorSetFrameList = std::vector<DescriptorSetList>;
		
		RenderService*			mRenderService;
		DescriptorSetAllocator* mDescriptorSetAllocator;	///< Allocator that is used to allocate new Descriptors
		VkDescriptorSetLayout	mLayout;					///< The layout that this cache is managing DescriptorSets for
		DescriptorSetList		mFreeList;					///< List of all available Descriptors
		DescriptorSetFrameList	mUsedList;					///< List of all used Descriptor, by frame index
	};

} // nap



