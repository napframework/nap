#include "renderable2dtextcomponent.h"
#include "renderglobals.h"
#include "renderservice.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>
#include <orthocameracomponent.h>
#include <nap/core.h>

// nap::Renderable2DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable2DTextComponent)
	RTTI_PROPERTY("Location",		&nap::Renderable2DTextComponent::mLocation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Orientation",	&nap::Renderable2DTextComponent::mOrientation,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",		&nap::Renderable2DTextComponent::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
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
		if (!setup(errorState))
			return false;
		
		// Copy flags
		Renderable2DTextComponent* resource = getComponent<Renderable2DTextComponent>();
		getMaterialInstance().setDepthMode(resource->mDepthMode);
		setOrientation(resource->mOrientation);
		setLocation(resource->mLocation);

		// Fetch render service
		mService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);
		
		return true;
	}


	void Renderable2DTextComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Compute model matrix
		glm::mat4x4 model_matrix;
		computeTextModelMatrix(model_matrix);

		// Compute new view matrix, don't scale or rotate!
		glm::vec3 cam_pos = math::extractPosition(viewMatrix);
		glm::mat4x4 view_matrix = glm::translate(glm::mat4(), cam_pos);

		// Call base class implementation based on given parameters
		RenderableTextComponentInstance::draw(renderTarget, commandBuffer, view_matrix, projectionMatrix, model_matrix);
	}


	glm::ivec2 Renderable2DTextComponentInstance::getTextPosition()
	{
		// Calculate offset
		const math::Rect& bounds = getBoundingBox();
		glm::ivec2 rvalue(0, mLocation.y);
		switch (mOrientation)
		{
		case utility::ETextOrientation::Left:
		{
			rvalue.x = mLocation.x - (int)(bounds.mMinPosition.x);
			break;
		}
		case utility::ETextOrientation::Center:
		{
			rvalue.x = mLocation.x - (int)(bounds.getWidth() / 2.0f);
			break;
		}
		case utility::ETextOrientation::Right:
		{
			rvalue.x = mLocation.x - (int)(bounds.getWidth());
			break;
		}
		default:
			assert(false);
		}

		// Extract component transform (x - y coordinates)
		glm::vec3 text_xform(0.0f, 0.0f, 0.0f);
		if (hasTransform())
		{
			text_xform = math::extractPosition(getTransform()->getGlobalTransform());
			rvalue.x += (int)(text_xform.x);
			rvalue.y += (int)(text_xform.y);
		}
		return rvalue;
	}


	nap::RenderableGlyph* Renderable2DTextComponentInstance::getRenderableGlyph(uint index, utility::ErrorState& error) const
	{
		assert(mFont != nullptr);
		return mFont->getOrCreateGlyphRepresentation<Renderable2DGlyph>(index, error);
	}


	bool Renderable2DTextComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	void Renderable2DTextComponentInstance::computeTextModelMatrix(glm::mat4x4& outMatrix)
	{
		// Get object space position based on orientation of text
		glm::ivec2 pos = getTextPosition();

		// Compose model matrix
		outMatrix = glm::translate(glm::mat4(),
		{
			(float)pos.x,
			(float)pos.y,
			0.0f
		});
	}


	void Renderable2DTextComponentInstance::draw(IRenderTarget& target)
	{
		// Create projection matrix
		glm::ivec2 size = target.getBufferSize();

		// Create projection matrix
		glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

		// Compute model matrix
		glm::mat4x4 model_matrix;
		computeTextModelMatrix(model_matrix);

		// Draw text in screen space
		RenderableTextComponentInstance::draw(target, mRenderService->getCurrentCommandBuffer(), glm::mat4(), proj_matrix, model_matrix);
	}
}