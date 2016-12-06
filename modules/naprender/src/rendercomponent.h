#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/coremodule.h>
#include <nap/attribute.h>

// Local Includes
#include "blendtype.h"

namespace nap
{
	/**
	 * Represents an object that can be rendered to screen
	 * or any other type of buffer. This is the base class
	 * for other render-able types.
	 * 
	 * You can override default draw behaviour by specializing the draw method
	 * OnDraw is called after draw by the render service that has access to
	 * this component
	 */
	class RenderableComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		RenderableComponent() = default;

		/**
		 * If the object should be drawn or not
		 */
		Attribute<bool>			draw = { this, "Draw", true };

		/**
		 * If alpha blending is enabled or not
		 * disabling alpha blending will invalidate the blend mode
		 */
		Attribute<bool>			blend = { this, "Blend", true };

		/**
		 * specifies first blend mode
		 */
		Attribute<BlendType>	firstBlendMode = { this, "FirstBlendMode", BlendType::SRC_ALPHA };

		/**
		 * specifies second blend mode
		 */
		Attribute<BlendType>	secondBlendMode = { this, "SecondBlendMode", BlendType::ONE_MINUS_SRC_ALPHA };

	protected:

	};
}
