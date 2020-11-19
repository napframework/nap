/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendercomponent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponent)
	RTTI_PROPERTY("Visible", &nap::RenderableComponent::mVisible, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponentInstance)
RTTI_END_CLASS

namespace nap
{
	bool RenderableComponentInstance::init(utility::ErrorState& errorState)
	{
		mVisible = getComponent<RenderableComponent>()->mVisible;
		return true;
	}


	void RenderableComponentInstance::draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!isVisible())
			return;
		onDraw(renderTarget, commandBuffer, viewMatrix, projectionMatrix);
	}
}
