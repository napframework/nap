/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <assert.h>
#include <vector>

// Local Includes
#include "vk_mem_alloc.h"

namespace nap
{
	/**
	 * Vulkan Image Data Structure.
	 * Binds image data, view and memory allocation information together for easy usage.
	 */
	struct NAPAPI ImageData
	{
		// Default Constructor
		ImageData() = default;

		// Constructor
		ImageData(uint viewCount) :
			mSubViews(viewCount, VK_NULL_HANDLE)			{ }

		/**
		 * @return Handle to Vulkan Image View
		 */
		VkImageView getView() const							{ return mView; }

		/**
		 * @return Handle to Vulkan Image View
		 */
		VkImageView getSubView(uint index) const			{ assert(index < mSubViews.size()); return mSubViews[index]; }

		/**
		 * @return Handle to Vulkan Image View
		 */
		uint getSubViewCount() const						{ return mSubViews.size(); }

		/**
		 * @return Handle to Vulkan Image Data
		 */
		VkImage getImage() const							{ return mImage; }

		/**
		 * @return Current Vulkan image layout.
		 */
		VkImageLayout getLayout() const						{ return mCurrentLayout; }

		/**
		 * Releases the image and view, resetting all the handles to null. Does not delete it.
		 */
		void release();

		VkImage							mImage = VK_NULL_HANDLE;						///< Vulkan Image
		VkImageView						mView = VK_NULL_HANDLE;							///< Vulkan Image view
		VmaAllocation					mAllocation = VK_NULL_HANDLE;					///< Vulkan single memory allocation
		VmaAllocationInfo				mAllocationInfo;								///< Vulkan memory allocation information
		VkImageLayout					mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;		///< Vulkan image layout
		std::vector<VkImageView>		mSubViews;										///< Vulkan Image views
	};
}
