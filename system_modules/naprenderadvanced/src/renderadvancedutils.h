/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>
#include <assert.h>

// Local Includes
#include "imagedata.h"
#include "bufferdata.h"
#include "surfacedescriptor.h"

namespace nap
{
	namespace utility
	{
		/**
		 * Creates a single or multi-sample renderpass based on rasterization samples and color/depth formats.
		 */
		bool NAPAPI createConsumeRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState);

		/**
		 * Creates a single or multi-sample depth-only renderpass based on rasterization samples and depth format.
		 */
		bool NAPAPI createDepthOnlyRenderPass(VkDevice device, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState);
	}
}
