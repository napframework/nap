#pragma once

#include "layoutcomponent.h"

namespace nap
{
	/**
	 * BoxLayout
	 *
	 * Lays out it's child components in a box, horizontally or vertically
	 */
	class BoxLayout : public LayoutComponent
	{
		RTTI_ENABLE_DERIVED_FROM(LayoutComponent)
	public:
		BoxLayout() : LayoutComponent() {}
		bool layout() override;
		Attribute<bool> autoStretch = { this, "AutoStretch", true };
		Attribute<bool> horizontal = { this, "Horizontal" , true };
		Attribute<bool> scaleToFit = { this, "ScaleToFit", false };
		Attribute<float> alignmentX = { this, "AlignmentX", 0.5f };
		Attribute<float> alignmentY = { this, "AlignmentY", 0.5f };
		Attribute<float> spacingX = { this, "SpacingX", 0.0f };
		Attribute<float> spacingY = { this, "SpacingY", 0.0f };
	private:
	};
}
