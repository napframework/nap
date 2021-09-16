/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderable3dtextcomponent.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::Renderable3DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable3DTextComponent)
	RTTI_PROPERTY("Normalize",	&nap::Renderable3DTextComponent::mNormalize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",	&nap::Renderable3DTextComponent::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::Renderable3DTextComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Renderable3DTextComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void Renderable3DTextComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool Renderable3DTextComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableTextComponentInstance::init(errorState))
			return false;

		// Setup base class
		if (!setup(errorState))
			return false;

		// Transform is required since text is rendered in 3D in the scene.
		if (!errorState.check(hasTransform(), "%s: missing transform component", mID.c_str()))
			return false;

		// Copy flags
		Renderable3DTextComponent* resource = getComponent<Renderable3DTextComponent>();
		getMaterialInstance().setDepthMode(resource->mDepthMode);
		normalizeText(resource->mNormalize);

		// Ensure that when we render normalized the scaling factor is up to date.
		if (isNormalized())
		{
			if (!errorState.check(computeNormalizationFactor(getText()), "%s: unable to calculate normalization factor, invalid text or empty text", mID.c_str()))
				return false;
		}

		return true;
	}


	bool Renderable3DTextComponentInstance::computeNormalizationFactor(const std::string& referenceText)
	{
		// Make sure there is text to analyze
		assert(mFont != nullptr);
		if (referenceText.empty())
			return false;

		// Get text bounding box
		nap::math::Rect bbox;
		mFont->getBoundingBox(referenceText, 1.0f, bbox);
		if (!bbox.hasWidth())
			return false;

		// Calculate normalization factor
		float max_size = bbox.getWidth() > bbox.getHeight() ? bbox.getWidth() : bbox.getHeight();
		mNormalizationFactor = 1.0f / max_size;
		return true;
	}


	nap::RenderableGlyph* Renderable3DTextComponentInstance::getRenderableGlyph(uint index, utility::ErrorState& error) const
	{
		assert(mFont != nullptr);
		return mFont->getOrCreateGlyphRepresentation<Renderable2DMipMapGlyph>(index, 1.0f, error);
	}


	void Renderable3DTextComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		assert(hasTransform());
		glm::mat4x4 model_matrix = getTransform()->getGlobalTransform();

		// Normalize text on request, this ensures the text is centered around 0 and 1 unit big
		const nap::math::Rect& bbox = getBoundingBox();
		if (mNormalize && bbox.hasWidth())
		{
			glm::vec3 scale(mNormalizationFactor,  mNormalizationFactor, 1.0f);
			glm::vec3 offset(0.0f, 0.0f, 0.0f);
			const nap::math::Rect& bbox = getBoundingBox();
			offset.x = 0.0f - ((scale.x * bbox.getWidth())  / 2.0f);

			glm::mat4x4  text_matrix = glm::translate(glm::mat4(), offset);
			text_matrix = glm::scale(text_matrix, scale);
			model_matrix = model_matrix * text_matrix;
		}
		Renderable3DTextComponentInstance::draw(renderTarget, commandBuffer, viewMatrix, projectionMatrix, model_matrix);
	}
}
