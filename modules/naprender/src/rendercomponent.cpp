// Local Includes
#include "rendercomponent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponentInstance)
RTTI_END_CLASS

namespace nap
{

	void RenderableComponentInstance::draw(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, int frameIndex, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!isVisible())
			return;

		onDraw(renderTarget, commandBuffer, frameIndex, viewMatrix, projectionMatrix);
	}
}
