#include "gridlayout.h"


RTTI_DEFINE(nap::GridLayout)

namespace nap
{

	bool GridLayout::layout()
	{
		if (colCount.getValue() < 1) colCount.setValue(1);
		if (rowCount.getValue() < 1) rowCount.setValue(1);

		std::vector<LayoutComponent*> layoutChildren;
		getLayoutChildren(layoutChildren);

		Rect innerBounds = getContentRect();

		float cellWidth = innerBounds.getWidth() / (float)colCount;
		float cellHeight = innerBounds.getHeight() / (float)rowCount;

		unsigned int i = 0;
		for (unsigned int i = 0; i < layoutChildren.size(); i++) 
		{
			int row = i % colCount;
			int col = i / colCount;

			LayoutComponent* layoutChild = layoutChildren[i];

			Rect rect;
			rect.setX(col * cellWidth);
			rect.setY(row * cellHeight);

			float x = margins.getValueRef().getLeft() + col * cellWidth;
			float y = margins.getValueRef().getTop() + row * cellHeight;
		}

		return true;
	}
}
