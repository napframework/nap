/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderable2dtextcomponent.h"
#include "renderglobals.h"
#include "renderservice.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>
#include <orthocameracomponent.h>
#include <nap/core.h>
#include <nap/logger.h>

// nap::Renderable2DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable2DTextComponent, "Render text at a specific location in screen space")
	RTTI_PROPERTY("Location",			&nap::Renderable2DTextComponent::mLocation,			nap::rtti::EPropertyMetaData::Default, "Pixel coordinates")
	RTTI_PROPERTY("Orientation",		&nap::Renderable2DTextComponent::mOrientation,		nap::rtti::EPropertyMetaData::Default, "Horizontal text alignment")
	RTTI_PROPERTY("DepthMode",			&nap::Renderable2DTextComponent::mDepthMode,		nap::rtti::EPropertyMetaData::Default, "Text depth mode")
	RTTI_PROPERTY("IgnoreTransform",	&nap::Renderable2DTextComponent::mIgnoreTransform,	nap::rtti::EPropertyMetaData::Default, "Ignores transform component when available")
	RTTI_PROPERTY("DPI Aware",			&nap::Renderable2DTextComponent::mDPIAware,			nap::rtti::EPropertyMetaData::Default, "Scale text based on display DPI")
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
		if (!RenderableTextComponentInstance::init(errorState))
			return false;

		// Fetch render service
		mService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);

		// Compute max font scaling factor based on highest display DPI value
		Renderable2DTextComponent* resource = getComponent<Renderable2DTextComponent>();
		mDPIAware = resource->mDPIAware && mService->getHighDPIEnabled();
		float fscale = 1.0f;
		if (mDPIAware)
		{
			for (const auto& display : mRenderService->getDisplays())
			{
				float dscale = display.getHorizontalDPI() / font::dpi;
				fscale = dscale > fscale ? dscale : fscale;
			}
		}

		// Init base class (setting up the plane glyph plane etc.)
		if (!setup(fscale, errorState))
			return false;
		
		// Copy flags
		getMaterialInstance().setDepthMode(resource->mDepthMode);
		setOrientation(resource->mOrientation);
		setLocation(resource->mLocation);
		mIgnoreTransform = resource->mIgnoreTransform;

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


	glm::ivec2 Renderable2DTextComponentInstance::getTextPosition(float scale)
	{
		// Calculate offset
		const math::Rect& bounds = getBoundingBox();
		glm::ivec2 rvalue(mLocation.x, mLocation.y);
		switch (mOrientation)
		{
			case utility::ETextOrientation::Left:
			{
				break;
			}
			case utility::ETextOrientation::Center:
			{
				rvalue.x -= static_cast<int>((bounds.getWidth() * scale) / 2.0f);
				break;
			}
			case utility::ETextOrientation::Right:
			{
				rvalue.x -= static_cast<int>(bounds.getWidth() * scale);
				break;
			}
			default:
			{
				assert(false);
			}
		}

		// Extract component transform (x - y coordinates)
		glm::vec3 text_xform(0.0f, 0.0f, 0.0f);
		if (hasTransform() && !mIgnoreTransform)
		{
			text_xform = math::extractPosition(getTransform()->getGlobalTransform());
			rvalue.x += (int)(text_xform.x);
			rvalue.y += (int)(text_xform.y);
		}
		return rvalue;
	}


	nap::RenderableGlyph* Renderable2DTextComponentInstance::getRenderableGlyph(uint index, float scale, utility::ErrorState& error) const
	{
		assert(mFont != nullptr);
		return mFont->getOrCreateGlyphRepresentation<Renderable2DGlyph>(index, scale, error);
	}


	bool Renderable2DTextComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	void Renderable2DTextComponentInstance::computeTextModelMatrix(glm::mat4x4& outMatrix)
	{
		float dpi_scale = 1.0f;
		auto* cur_window = mRenderService->getCurrentRenderWindow();

		// Compute dpi scaling factor, based on highest dpi scaling value, always < 1
		// Note that current window is unavailable when rendering headless
		if (mDPIAware && cur_window != nullptr)
		{
			auto* display = mRenderService->findDisplay(*cur_window);
			assert(display != nullptr);
			dpi_scale = (1.0f / getDPIScale()) * (math::max<float>(display->getHorizontalDPI(), font::dpi) / font::dpi);
		}

		// Get object space position based on orientation of text
		glm::ivec2 pos = getTextPosition(dpi_scale);
		outMatrix = glm::translate(glm::mat4(), { (float)pos.x, (float)pos.y, 0.0f });
		outMatrix = glm::scale(outMatrix, { dpi_scale, dpi_scale, 1.0f });
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
