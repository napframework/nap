#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/serviceablecomponent.h>
#include <nap/coreattributes.h>
#include <nap/attribute.h>
#include <nap/componentdependency.h>

// Local Includes
#include "napofblendtype.h"
#include "napoftransform.h"
#include "napofmaterial.h"

namespace nap
{
    /**
     * Renderable NAP component
     *
     * Any derived component will be able to draw itself using the OF render engine
     * The render engine will first call onDraw after binding the right shader
     * The render engine will then call onPostDraw without binding a shader
     */
	class OFRenderableComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

	public:
		// Default constructor
		OFRenderableComponent()		{ }

		// Render Attributes
		Attribute<bool>				mEnableDrawing		= { this, "Draw", true };
		Attribute<bool>				mAlphaBlending		= { this, "BlendAlpha", true };
		Attribute<OFBlendType>		mFirstBlendMode		= { this, "FirstBlendMode", OFBlendType::SRC_ALPHA };
		Attribute<OFBlendType>		mSecondBlendMode	= { this, "SecondBlendMode", OFBlendType::ONE_MINUS_SRC_ALPHA };

		// Render call
		void						draw();
		virtual void				onDraw() = 0;
		virtual void				onPostDraw() {};

		ComponentDependency<OFTransform>		mTransform			= { this };
		ComponentDependency<OFMaterial>			mMaterial			= { this };
	};
}

RTTI_DECLARE_BASE(nap::OFRenderableComponent)
