/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>

// Local Includes
#include "vk_mem_alloc.h"

namespace nap
{
	/**
	 * Vulkan Buffer Data Structure
	 * Binds a buffer, usage information, memory allocation and allocation information together.
	 */
	struct NAPAPI BufferData
	{
		// Default constructor
		BufferData() = default;

		/**
		 * Releases the buffer, resetting all the handles to null. Does not delete it.
		 */
		void					release();

		VmaAllocation			mAllocation = VK_NULL_HANDLE;				///< Vulkan memory allocation handle
		VmaAllocationInfo		mAllocationInfo;							///< Vulkan allocation information
		VkBufferUsageFlags		mUsage = 0;									///< Usage flags
		VkBuffer				mBuffer = VK_NULL_HANDLE;					///< Vulkan buffer
	};
}
