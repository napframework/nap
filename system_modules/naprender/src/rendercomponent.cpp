/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendercomponent.h"
#include "renderservice.h"
#include "rendertag.h"

// NAP Includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponent)
	RTTI_PROPERTY("Visible", &nap::RenderableComponent::mVisible, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Tags", &nap::RenderableComponent::mTags, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LayerRegistry", &nap::RenderableComponent::mLayerRegistry, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Layer", &nap::RenderableComponent::mLayer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableComponentInstance)
RTTI_END_CLASS

namespace nap
{
	RenderableComponentInstance::RenderableComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<nap::RenderService>())
	{}


	bool RenderableComponentInstance::init(utility::ErrorState& errorState)
	{
		const auto& resource = getComponent<RenderableComponent>();
		mVisible = resource->mVisible;

		// Check if this component is assigned to a layer
		if (resource->mLayer != nullptr)
		{
			// Bail early if there is no layer registry
			if (!errorState.check(resource->mLayerRegistry != nullptr, "%s: Layer specified without registry. Please provide a registry to ensure it is initialized before the layer.", resource->mID.c_str()))
				return false;

			// Store the layer index
			mRenderLayer = resource->mLayer->getIndex();
		}

		// Ensure there are no tag entries that are nullptrs
		for (const auto& tag : resource->mTags)
		{
			// Ensure tag is present
			if (!errorState.check(tag != nullptr, "%s: Empty tag entry encountered", resource->mID.c_str()))
				return false;

			// Add to render mask
			mRenderMask |= tag->getMask();
		}
		return true;
	}


	void RenderableComponentInstance::draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!isVisible())
			return;
		onDraw(renderTarget, commandBuffer, viewMatrix, projectionMatrix);
	}
}
