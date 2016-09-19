#pragma once

#include "layoutcomponent.h"

namespace nap
{


	// Divides the layout in two sections, the splitter location set as a fraction (0 - 1)
	class SplitLayout : public LayoutComponent
	{


	public:
		SplitLayout() : LayoutComponent() {}


		bool layout() override
		{
			auto layoutChildren = getLayoutChildren();
			if (layoutChildren.size() != 2) 
			{
				nap::Logger::fatal("Should have exaclty 2 layout children, got %d, not doing layout.",
								   layoutChildren.size());
				return true;
			}

			auto firstChild = layoutChildren[0];
			auto secondChild = layoutChildren[1];
			Rect contentRect = getContentRect();

			if (horizontalOrientation.getValueRef()) 
			{

				float leftSize = contentRect.getWidth() * splitterLocation.getValueRef();
				float rightSize = contentRect.getWidth() - leftSize;

				firstChild->bounds.getValueRef().setWidth(leftSize);
				//secondChild->bounds.getValueRef().setHeight()
			} 
			else 
			{
				float topSize = contentRect.getHeight() * splitterLocation.getValueRef();
			}
			return true;
		}


	private:
		Attribute<bool> horizontalOrientation = {this, "horizontalOrientation", false};
		Attribute<float> splitterLocation = {this, "splitterLocation", 0.5f};
		Attribute<float> spacing = {this, "spacing", 10};
	};
}