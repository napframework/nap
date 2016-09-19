#include "fractionlayout.h"

RTTI_DEFINE(nap::FractionLayout)

namespace nap {

	bool FractionLayout::layout()
	{
		Rect bounds = getContentRect();

		for (auto& child : getLayoutChildren())
		{
			Rect childBounds = child->bounds.getValue();

			if (hasWidth(child) && hasHeight(child))
			{
				childBounds.setWidth(getWidth(child) * bounds.getWidth());
				childBounds.setHeight(getHeight(child) * bounds.getHeight());
			}
			else if (hasWidth(child) && !hasHeight(child))
			{
				childBounds.setWidth(getWidth(child) * bounds.getWidth());
				if (hasAspectRatio(child))
				{
					childBounds.setHeight(childBounds.getWidth() / getAspectRatio(child));
				}
			}
			else if (!hasWidth(child) && hasHeight(child))
			{
				childBounds.setHeight(getHeight(child) * bounds.getHeight());
				if (hasAspectRatio(child))
					childBounds.setWidth(childBounds.getHeight() * getAspectRatio(child));
			}

			float fracX = alignmentsX.getValueRef()[ObjectPath(child)];
			float fracY = alignmentsY.getValueRef()[ObjectPath(child)];

			float x = (bounds.getWidth() - childBounds.getWidth()) * fracX;
			float y = (bounds.getHeight() - childBounds.getHeight()) * fracY;

			child->bounds.setValue({ x, y, childBounds.getWidth(), childBounds.getHeight() });
		}
		return true;
	}


	void FractionLayout::setAlignment(LayoutComponent* comp, float x, float y)
	{
		std::string compPath = ObjectPath(comp);
		alignmentsX.getValueRef()[compPath] = x;
		alignmentsY.getValueRef()[compPath] = y;
	}


	void FractionLayout::setSize(LayoutComponent* comp, float width, float height)
	{
		std::string compPath = ObjectPath(comp);
		widths.getValueRef()[compPath] = width;
		heights.getValueRef()[compPath] = height;
	}

	void FractionLayout::setSizeUsingWidth(LayoutComponent* comp, float width, float aspectRatio)
	{
		std::string compPath = ObjectPath(comp);
		widths.getValueRef()[compPath] = width;
		aspectRatios.getValueRef()[compPath] = aspectRatio;
	}


	void FractionLayout::setSizeUsingHeight(LayoutComponent* comp, float height, float aspectRatio)
	{
		std::string compPath = ObjectPath(comp);
		heights.getValueRef()[compPath] = height;
		aspectRatios.getValueRef()[compPath] = aspectRatio;
	}

}
