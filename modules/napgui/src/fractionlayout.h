#pragma once

#include "layoutcomponent.h"

namespace nap
{

	// Layout each child inside the parent based on fraction per axis
	// Set x to 0.0 to stick the component to the left,
	// set it to 1.0 to align it right.
	// A value of 0.5 will align it in the middle
	class FractionLayout : public LayoutComponent
	{
		RTTI_ENABLE_DERIVED_FROM(LayoutComponent)
	public:
		FractionLayout() : LayoutComponent() {}

		bool layout() override;

		void setAlignment(LayoutComponent* comp, float x, float y);

		void setSize(LayoutComponent* comp, float width, float height);

		// sets the size of a component using the width as a fraction of the parent's size and the aspect ratio of the component's size (width/height)
		void setSizeUsingWidth(LayoutComponent* comp, float width, float aspectRatio);
		// sets the size of a component using the height as a fraction of the parent's size and the aspect ratio of the component's size (width/height)
		void setSizeUsingHeight(LayoutComponent* comp, float height, float aspectRatio);

		Attribute<FloatMap> alignmentsX = { this, "offsetsX" }; 
		Attribute<FloatMap> alignmentsY = { this, "offsetsY" };

		// if only this height is specified the height fraction will be used for the width in combination with the aspect ratio and vice versa
		// if neither width nor height is specified the original bounds in pixels will be used
		Attribute<FloatMap> widths = { this, "widths" };
		Attribute<FloatMap> heights = { this, "heights" };

		// The ratio of width / height of the layout components
		Attribute<FloatMap> aspectRatios = { this, "aspectRatios" };

	private:
		bool hasWidth(LayoutComponent* comp) { return widths.getValueRef().find(ObjectPath(comp)) != widths.getValueRef().end(); }
		bool hasHeight(LayoutComponent* comp) { return heights.getValueRef().find(ObjectPath(comp)) != heights.getValueRef().end(); }
		bool hasAspectRatio(LayoutComponent* comp) { return aspectRatios.getValueRef().find(ObjectPath(comp)) != aspectRatios.getValueRef().end(); }
		float getWidth(LayoutComponent* comp) { return widths.getValueRef()[ObjectPath(comp)]; }
		float getHeight(LayoutComponent* comp) { return heights.getValueRef()[ObjectPath(comp)]; }
		float getAspectRatio(LayoutComponent* comp) { return aspectRatios.getValueRef()[ObjectPath(comp)]; }
	};
}

RTTI_DECLARE(nap::FractionLayout)