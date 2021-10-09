/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "descriptorsetallocator.h"
#include <assert.h>

namespace nap
{
	DescriptorSetAllocator::DescriptorSetAllocator(VkDevice device) :
		mDevice(device)
	{ }


	DescriptorSetAllocator::~DescriptorSetAllocator()
	{
		for (auto kvp : mDescriptorPools)
		{
			for (DescriptorPool& pool : kvp.second)
			{
				vkFreeDescriptorSets(mDevice, pool.mPool, pool.mAllocatedDescriptorSets.size(), pool.mAllocatedDescriptorSets.data());
				vkDestroyDescriptorPool(mDevice, pool.mPool, nullptr);
			}
		}
	}


	VkDescriptorSet DescriptorSetAllocator::allocate(VkDescriptorSetLayout layout, int numUBODescriptors, int numSamplerDescriptors)
	{
		uint64_t key = ((uint64_t)numUBODescriptors) << 32 | numSamplerDescriptors;

		DescriptorPool* free_descriptor_pool = nullptr;
		DescriptorPoolMap::iterator pos = mDescriptorPools.find(key);

		// See if we already have pool(s) for this combination of UBOs/samplers
		if (pos != mDescriptorPools.end())
		{
			// We do, now search within the list of pools for one that has empty space, starting with the last one
			std::vector<DescriptorPool>& pools = pos->second;
			for (int i = pools.size() - 1; i >= 0; --i)
			{
				DescriptorPool& descriptor_pool = pools[i];

				if (descriptor_pool.mAllocatedDescriptorSets.size() < descriptor_pool.mMaxNumSets)
				{
					free_descriptor_pool = &descriptor_pool;
					break;
				}
			}
		}

		// If there was no pool, or none of the pools had room, we need to create a new one
		if (free_descriptor_pool == nullptr)
		{
			// Make room for 100 DescriptorSets within a single pool
			int maxSets = 100;

			// We need to specify how many subresources to allocate within the pool. We multiply the amount of required UBOs with the amount
			// of sets to come up with the correct amount of subresources
			std::vector<VkDescriptorPoolSize> pool_sizes;
			if (numUBODescriptors != 0)
				pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)(numUBODescriptors * maxSets) });

			if (numSamplerDescriptors != 0)
				pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)(numSamplerDescriptors * maxSets) });

			// TODO: With the addition of compute shaders we may also need to allocate for storage buffers (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			// and take into account their appropriate pool sizes

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = pool_sizes.size();
			poolInfo.pPoolSizes = pool_sizes.data();
			poolInfo.maxSets = maxSets;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			DescriptorPool new_descriptor_pool;
			new_descriptor_pool.mMaxNumSets = maxSets;

			VkResult result = vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &new_descriptor_pool.mPool);
			assert(result == VK_SUCCESS);

			mDescriptorPools[key].push_back(new_descriptor_pool);

			free_descriptor_pool = &mDescriptorPools[key].back();
		}

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = free_descriptor_pool->mPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptor_set = nullptr;
		vkAllocateDescriptorSets(mDevice, &allocInfo, &descriptor_set);
		assert(descriptor_set != nullptr);
		free_descriptor_pool->mAllocatedDescriptorSets.push_back(descriptor_set);
		return descriptor_set;
	}

}

