/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <unordered_map>
#include <utility/dllexport.h>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace nap
{
	/**
	 * Allocates DescriptorSets from a pool. Each pool is bound to a specific combination of UBOs and samplers,
	 * as allocates resources for each UBO and sampler as well as the DescriptorSet itself. Any shader that has 
	 * the same amount of UBOs and samplers can therefore allocate from the same pool.
	 *
	 * So, we maintain a map that holds a key that is a combination of sampler/UBO count. As a value, there is a 
	 * list of pools, because pools are preallocated with a fixed size of sets to allocate from. If the pool is
	 * full, we need to allocate a new pool.
	 *
	 * So essentially we're managing a pool of DescriptorSetPools here.
	 */
	class NAPAPI DescriptorSetAllocator final
	{
	public:
		/**
		 * @param device resource to allocate descriptors from
		 */
		DescriptorSetAllocator(VkDevice device);

		// Destructor
		~DescriptorSetAllocator();

		/**
		 * Allocate a DescriptorSetLayout that is compatible with VkDescriptorSetLayout (same amount of UBOs and samplers).
		 * @param layout layout of the descriptor set
		 * @param numUBODescriptors number of uniform buffer object descriptors
		 * @param numHBODescriptors number of storage buffer object descriptors
		 * @param numSamplerDescriptors number of sampler descriptors.
		 */
		VkDescriptorSet allocate(VkDescriptorSetLayout layout, int numUBODescriptors, int numHBODescriptors, int numSamplerDescriptors);

	private:
		/**
		 * Wrapper around VkDescriptorPool that maintains a use count for amount of sets that are allocated within the pool.
		 */
		struct DescriptorPool
		{
			using DescriptorSetList = std::vector<VkDescriptorSet>;
			VkDescriptorPool	mPool = VK_NULL_HANDLE;					///< The managed descriptor pool
			DescriptorSetList	mAllocatedDescriptorSets;				///< The sets that have been allocated from this pool
			int					mMaxNumSets = 0;						///< Maximum number of sets that can be allocated from the pool
		};

		using DescriptorPoolMap = std::unordered_map<uint64_t, std::vector<DescriptorPool>>;
		VkDevice				mDevice;
		DescriptorPoolMap		mDescriptorPools;
	};
} // nap



