#pragma once

#include "utility/dllexport.h"
#include "glm/glm.hpp"
#include "materialcommon.h"
#include "vulkan/vulkan_core.h"

namespace nap
{
	/**
	 * 
	 */
	class NAPAPI IRenderTarget
	{
	public:
		virtual void beginRendering() = 0;
		virtual void endRendering() = 0;
		virtual const glm::ivec2 getBufferSize() const = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual const glm::vec4& getClearColor() const = 0;
		virtual ECullWindingOrder getWindingOrder() const = 0;
		virtual VkFormat getColorFormat() const = 0;
		virtual VkFormat getDepthFormat() const = 0;
		virtual VkRenderPass getRenderPass() const = 0;
		virtual VkSampleCountFlagBits getSampleCount() const = 0;
		virtual bool getSampleShadingEnabled() const = 0;
	};
}
