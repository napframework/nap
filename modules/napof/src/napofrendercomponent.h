#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// NAP Includes
#include <nap/serviceablecomponent.h>
#include <nap/coremodule.h>
#include <nap/attribute.h>

// OF Includes
#include <napofblendtype.h>
#include <nap/componentdependency.h>
#include <napoftransform.h>
#include <napofmaterial.h>

namespace nap
{
	/**
	@brief Renderable NAP component
	**/
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
