#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/coremodule.h>
#include <nap/attribute.h>

// Local Includes
#include "blendtype.h"
#include "material.h"

namespace nap
{
	/**
	 * Represents an object that can be rendered to screen
	 * or any other type of buffer. This is the base class
	 * for other render-able types.
	 * 
	 * You can override default draw behavior by specializing the draw method
	 * OnDraw is called after draw by the render service that has access to
	 * this component
	 */
	class RenderableComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		RenderableComponent();

		/**
		 * If the object should be drawn or not
		 */
		Attribute<bool> enabled =					{ this, "Enabled", true };

		/**
		 * If alpha blending is enabled or not
		 * disabling alpha blending will invalidate the blend mode
		 */
		Attribute<bool>	blend =						{ this, "Blend", true };

		/**
		 * Specifies the first OpenGL blend mode to use when rendering
		 */
		Attribute<BlendType> firstBlendMode =		{ this, "FirstBlendMode", BlendType::SRC_ALPHA };

		/**
		 * Specifies the second OpenGL blend mode to use when rendering
		 */
		Attribute<BlendType> secondBlendMode =		{ this, "SecondBlendMode", BlendType::ONE_MINUS_SRC_ALPHA };

		/**
		 * Draws the data to the currently active render target
		 * Binds the material and calls onDraw afterwards.
		 * Override if you want to customize material and binding behavior
		 */
		virtual void draw();

		/**
		 * Always called after draw and needs to be implemented
		 * Handles drawing of object knowing the right materials have been assigned
		 */
		virtual void onDraw() = 0;

		/**
		 * Called after draw
		 * useful for performing late drawing operations based
		 * on the data represented by the render component
		 * default implementation does nothing
		 */
		virtual void onPostDraw()		{ }

		/**
		 * Link to material used for drawing
		 */
		ObjectLinkAttribute material = { this, "material", RTTI_OF(Material) };

		/**
		 * @return link as material, nullptr if not found
		 */
		Material* getMaterial();
	};
}

RTTI_DECLARE_BASE(nap::RenderableComponent)
