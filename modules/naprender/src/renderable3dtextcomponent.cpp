#include "renderable3dtextcomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::Renderable3DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable3DTextComponent)
	RTTI_PROPERTY("Normalize",	&nap::Renderable3DTextComponent::mNormalize,	nap::rtti::EPropertyMetaData::Default)
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

		if (!errorState.check(hasTransform(), "%s doesn't have a transform component", getEntityInstance()->mID.c_str()))
			return false;

		// Copy flags
		normalizeText(getComponent<Renderable3DTextComponent>()->mNormalize);

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
		mFont->getBoundingBox(referenceText, bbox);
		if (!bbox.hasWidth())
			return false;

		// Calculate normalization factor
		mNormalizationFactor = 1.0f / bbox.getWidth();
		return true;
	}


	nap::RenderableGlyph* Renderable3DTextComponentInstance::getRenderableGlyph(uint index, utility::ErrorState& error) const
	{
		assert(mFont != nullptr);
		return mFont->getOrCreateGlyphRepresentation<Renderable2DMipMapGlyph>(index, error);
	}


	void Renderable3DTextComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
			offset.x = 0.0f - ((scale.x * bbox.getWidth()) / 2.0f);

			glm::mat4x4  text_matrix = glm::translate(identityMatrix, offset);
			text_matrix = glm::scale(text_matrix, scale);
			model_matrix = model_matrix * text_matrix;
		}
		Renderable3DTextComponentInstance::draw(viewMatrix, projectionMatrix, model_matrix);
	}
}