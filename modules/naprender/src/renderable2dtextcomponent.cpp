#include "renderable2dtextcomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>

// nap::Renderable2DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable2DTextComponent)
	RTTI_PROPERTY("Orientation", &nap::Renderable2DTextComponent::mOrientation, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::Renderable2DTextComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Renderable2DTextComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool Renderable2DTextComponentInstance::init(utility::ErrorState& errorState)
	{
		// Init base class (setting up the plane glyph plane etc.)
		if (!RenderableTextComponentInstance::init(errorState))
			return false;

		// Copy orientation
		setOrientation(getComponent<Renderable2DTextComponent>()->mOrientation);

		return true;
	}


	void Renderable2DTextComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Compute model matrix
		glm::mat4x4 model_matrix;
		computeTextModelMatrix({0, 0}, model_matrix);

		// Call base class implementation based on given parameters
		RenderableTextComponentInstance::draw(viewMatrix, projectionMatrix, model_matrix);
	}


	glm::ivec2 Renderable2DTextComponentInstance::getTextPosition(const glm::ivec2& origin)
	{
		// Calculate offset
		const math::Rect& bounds = getBoundingBox();
		glm::ivec2 rvalue(0, origin.y);
		switch (mOrientation)
		{
		case utility::ETextOrientation::Left:
		{
			rvalue.x = origin.x - (int)(bounds.mMinPosition.x);
			break;
		}
		case utility::ETextOrientation::Center:
		{
			rvalue.x = origin.x - (int)(bounds.getWidth() / 2.0f);
			break;
		}
		case utility::ETextOrientation::Right:
		{
			rvalue.x = origin.x - (int)(bounds.getWidth());
			break;
		}
		default:
			assert(false);
		}
		return rvalue;
	}


	nap::RenderableGlyph* Renderable2DTextComponentInstance::getRenderableGlyph(uint index, utility::ErrorState& error) const
	{
		assert(mFont != nullptr);
		return mFont->getOrCreateGlyphRepresentation<Renderable2DGlyph>(index, error);
	}


	void Renderable2DTextComponentInstance::computeTextModelMatrix(const glm::ivec2& coordinates, glm::mat4x4& outMatrix)
	{
		// Get object space position based on orientation of text
		glm::ivec2 pos = getTextPosition(coordinates);

		// Extract component transform (x - y coordinates)
		glm::vec3 text_xform(0.0f, 0.0f, 0.0f);
		if (hasTransform())
			text_xform = math::extractPosition(getTransform()->getGlobalTransform());

		// Compose model matrix
		outMatrix = glm::translate(identityMatrix,
		{
			(float)pos.x + text_xform.x,
			(float)pos.y + text_xform.y,
			0.0f
		});
	}


	void Renderable2DTextComponentInstance::draw(const glm::ivec2& coordinates, const opengl::BackbufferRenderTarget& target)
	{
		// Create projection matrix
		glm::mat4 proj_matrix = glm::ortho(0.0f, (float)target.getSize().x, 0.0f, (float)target.getSize().y);
		
		// Compute model matrix
		glm::mat4x4 model_matrix;
		computeTextModelMatrix(coordinates, model_matrix);

		// Draw text in screen space
		RenderableTextComponentInstance::draw(identityMatrix, proj_matrix, model_matrix);
	}
}