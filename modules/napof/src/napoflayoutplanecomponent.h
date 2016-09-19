#pragma once

#include <nap.h>

#include <layoutcomponent.h>

#include "napofsimpleshapecomponent.h"
#include "napoftransform.h"


namespace nap 
{
	// Defines a drawable plane that listens to layout changes
	// The transform and bounds are updated relative to the layout found on the parent entity 
	class OFLayoutPlaneComponent : public OFPlaneComponent 
	{
		RTTI_ENABLE_DERIVED_FROM(OFPlaneComponent)
	public:
		OFLayoutPlaneComponent();

		Slot<LayoutComponent&> layoutSlot = { this, &OFLayoutPlaneComponent::onLayout };

		void onPostDraw() override;

        Point layoutToWorld(const Point& p);

		Attribute<bool> debugDraw = { this, "DebugDraw", false };
		Attribute<float> scaleOverride = { this, "ScaleOverride", 1 };
	private:
		void onLayout(LayoutComponent& layout);
		void onDebugDrawChanged(bool enabled);

		ComponentDependency<LayoutComponent> mLayoutComponent = { this };
		ComponentDependency<OFTransform> mTransform = { this };
	};
}

RTTI_DECLARE(nap::OFLayoutPlaneComponent)