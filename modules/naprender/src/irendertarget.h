/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "utility/dllexport.h"
#include "glm/glm.hpp"
#include "materialcommon.h"
#include "vulkan/vulkan_core.h"

namespace nap
{
	/**
	 * Interface for all render targets, including nap::RenderWindow and nap::RenderTarget.
	 */
	class NAPAPI IRenderTarget
	{
	public:
		/**
		 * Should start the render pass
		 */
		virtual void beginRendering() = 0;

		/**
		 * Should end the render pass
		 */
		virtual void endRendering() = 0;

		/**
		 * @return the size of the buffer in pixels
		 */
		virtual const glm::ivec2 getBufferSize() const = 0;

		/**
		 * Allows for setting the clear color
		 */
		virtual void setClearColor(const glm::vec4& color) = 0;

		/**
		 * @return the clear color
		 */
		virtual const glm::vec4& getClearColor() const = 0;

		/**
		 * @return triangle winding order
		 */
		virtual ECullWindingOrder getWindingOrder() const = 0;

		/**
		 * @return used color format
		 */
		virtual VkFormat getColorFormat() const = 0;

		/**
		 * @return used depth format
		 */
		virtual VkFormat getDepthFormat() const = 0;

		/**
		 * @return the render pass
		 */
		virtual VkRenderPass getRenderPass() const = 0;

		/**
		 * @return number of samples
		 */
		virtual VkSampleCountFlagBits getSampleCount() const = 0;

		/**
		 * @return if sample based shading is enabled
		 */
		virtual bool getSampleShadingEnabled() const = 0;
	};
}
