/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Internal includes
#include "vk_mem_alloc.h"
#include "renderutils.h"
#include "gpudatabuffer.h"

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
	class UniformOpaqueBufferObject;
	class SamplerInstance;
	class RenderService;
	class DescriptorSetAllocator;

	/** 
	 * Wrapper around VkDescriptorSet that also contains the allocated buffers for a DescriptorSet.
	 */
	struct DescriptorSet
	{
		VkDescriptorSetLayout				mLayout;
		VkDescriptorSet						mSet;
		std::vector<BufferData>				mBuffers;
		const std::vector<BufferData>& getBuffers() const { return mBuffers; }
	};


	/** 
	 * Responsible for caching DescriptorSets and allocating them when the DescriptorSet is not in the cache.
	 * A DescriptorSetAllocator allocates DescriptorSets for a specific VkDescriptorSetLayout. Any DescriptorSet
	 * that is returned will be compatible with that VkDescriptorSetLayout.
	 *
	 * Internally it will maintain a free-list of available DescriptorSets. When a VkDescriptorSet is acquired,
	 * it is marked for use by that frame (the current RenderService frame is used). When a frame is fully  
	 * completed, release should be called for that frame so that the resources are return to the free-list, to 
	 * be used by subsequent frames.
	 */
	class NAPAPI BaseDescriptorSetCache
	{
	public:
		BaseDescriptorSetCache(RenderService& renderService, VkDescriptorSetLayout layout, DescriptorSetAllocator& descriptorSetAllocator);
		~BaseDescriptorSetCache();

		/**
		 * Acquires a DescriptorSet from the cache (or allocated it if not in the cache). For new DescriptorSets,
		 * also allocates Buffers for each UBO from the global Vulkan allocator. The result is a DescriptorSet
		 * that is fully compatible with the DescriptorSetLayout.
		 * @param uniformBufferObjects The list of UBOs for this DescriptorSet.
		 * @param numSamplers The number of samplers for this DescriptorSet
		 * @return A DescriptorSet that is compatible with the VkDescriptorLayout that was passed upon creation.
		 */
		virtual const DescriptorSet& acquire(const std::vector<UniformBufferObject>& uniformBufferObjects, int numSamplers) = 0;

		virtual void release(int frameIndex) { };

	protected:
		using DescriptorSetList = std::list<DescriptorSet>;
		using DescriptorSetFrameList = std::vector<DescriptorSetList>;

		RenderService*			mRenderService;
		DescriptorSetAllocator* mDescriptorSetAllocator;	///< Allocator that is used to allocate new Descriptors
		VkDescriptorSetLayout	mLayout;					///< The layout that this cache is managing DescriptorSets for
	};


	class NAPAPI DescriptorSetCache : public BaseDescriptorSetCache
	{
	public:
		DescriptorSetCache(RenderService& renderService, VkDescriptorSetLayout layout, DescriptorSetAllocator& descriptorSetAllocator);
		~DescriptorSetCache();

		/**
		 * Acquires a DescriptorSet from the cache (or allocated it if not in the cache). For new DescriptorSets,
		 * also allocates Buffers for each UBO from the global Vulkan allocator. The result is a DescriptorSet
		 * that is fully compatible with the DescriptorSetLayout.
		 * @param uniformBufferObjects The list of UBOs for this DescriptorSet.
		 * @param numSamplers The number of samplers for this DescriptorSet
		 * @return A DescriptorSet that is compatible with the VkDescriptorLayout that was passed upon creation.
		 */
		virtual const DescriptorSet& acquire(const std::vector<UniformBufferObject>& uniformBufferObjects, int numSamplers) override;

		/**
		 * Releases all DescriptorSets to the internal pool for use by other frames. 
		 * @param frameIndex The frame index that was completed by the render system and for which no resources are in use anymore.
		 */
		void release(int frameIndex);

	private:
		DescriptorSetList		mFreeList;					///< List of all available Descriptors
		DescriptorSetFrameList	mUsedList;					///< List of all used Descriptor, by frame index
	};

	/**
	 * 
	 */
	class NAPAPI StaticDescriptorSetCache : public BaseDescriptorSetCache
	{
	public:
		StaticDescriptorSetCache(RenderService& renderService, VkDescriptorSetLayout layout, DescriptorSetAllocator& descriptorSetAllocator);
		~StaticDescriptorSetCache();

		/**
		 * Acquires a DescriptorSet from the cache (or allocated it if not in the cache). For new DescriptorSets,
		 * also allocates Buffers for each UBO from the global Vulkan allocator. The result is a DescriptorSet
		 * that is fully compatible with the DescriptorSetLayout.
		 * @param uniformBufferObjects The list of UBOs for this DescriptorSet.
		 * @param numSamplers The number of samplers for this DescriptorSet
		 * @return A DescriptorSet that is compatible with the VkDescriptorLayout that was passed upon creation.
		 */
		virtual const DescriptorSet& acquire(const std::vector<UniformBufferObject>& uniformBufferObjects, int numSamplers) override;

	private:
		std::vector<BufferData> mStagingBuffers;

		DescriptorSet			mDescriptorSet;
		bool					mAllocated = false;
	};

} // nap
