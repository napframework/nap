#pragma once

#include "rect.h"
#include <nap.h>

namespace nap
{
	// All metrics in cm
	class LayoutComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		LayoutComponent() : Component(), bounds(this, "Bounds", Rect()) {
			added.connect([&](Object& owner) 
			{
				ppcmX.setValue(*(getRoot()->getOrCreateAttribute<float>("ppcmX", 36)));
				ppcmY.setValue(*(getRoot()->getOrCreateAttribute<float>("ppcmY", 36)));
			});
		}
		LayoutComponent(const LayoutComponent& other) = delete;

		LayoutComponent* getParentLayout();
		void getLayoutChildren(std::vector<LayoutComponent*>& outChildren);
		std::vector<LayoutComponent*> getLayoutChildren();

		// Let's each sub class implement it's own layout behavior
		// The return value specifies if children should have it's layout updated or not
		virtual bool layout() { return true; }

		// Calls layout on each layout component in the tree
		void validateLayout();
		
		// Depth offset getters / setters
		void setDepthOffset(float d) { mDepthOffset.setValue(d); }
		float getDepthOffset() { return mDepthOffset.getValue(); }

		// Return the bounds minus the margins
		Rect getContentRect() const;

		const Rect& getBounds() const { return bounds.getValue();  }
		const Margins& getMargins() const { return margins.getValue(); }

		float getDepth();

		// The item bounds in cm's
		Attribute<Rect> bounds = { this, "Bounds", { 0, 0, 100, 100} };
		
		// The item's margins (relative to bounds) in cm's
		Attribute<Margins> margins = { this, "Margins", {0,0,0,0} };

		// Layout width and ratio (used by the fraction layout, why is that here?)
		Attribute<float> width = { this, "FractionWidth", 0.5f };
		Attribute<float> ratio = { this, "Ratio", 1.0f };

		// Pixels per centimeter
		ObjectLinkAttribute ppcmX = { this, "ppcmX", RTTI_OF(Attribute<float>) };
		ObjectLinkAttribute ppcmY = { this, "ppcmY", RTTI_OF(Attribute<float>) };

		// Relative depth offset this item should get relative to it's parent
		Attribute<float> mDepthOffset = { this, "depthOffset", 0 };

		// Map point p from the parent space into this layout's space
		Point mapFromParent(const Point& p);

		// Map point p from this layout's space into its parent's space
		Point mapToParent(const Point& p);

		// Map point p from the root/global space into this layout's space
		Point mapFromRoot(const Point &p);

		// Map point p from this layout's space into the root/global space
		Point mapToRoot(const Point &p);

        // Map point p from the provided source layout to this layout
        Point mapFromLayout(LayoutComponent &sourceLayout, const Point &p);


		// Converts pixel coordinate to cm's
		float pixelsToCmX(float inPixel);
		float pixelsToCmY(float inPixel);

		Signal<LayoutComponent&> layoutChanged;

		LayoutComponent& operator=(const LayoutComponent&) = delete;

	private:
		float mDepth = 1;
		static const float mDepthIncrement;
		void updateLayoutPlane();
	};

}
