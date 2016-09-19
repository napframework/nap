#include "boxlayout.h"


RTTI_DEFINE(nap::BoxLayout)

namespace nap
{

	bool BoxLayout::layout()
	{
		auto bounds = getContentRect();

		std::vector<LayoutComponent*> children = getLayoutChildren();
		int childCount = children.size();

		if (autoStretch.getValue()) // Stretch children
		{
			if (horizontal.getValue()) 
			{
				for (int i = 0; i < childCount; i++) 
				{
					LayoutComponent* child = children[i];

					float width = bounds.getWidth() / (float)childCount;
					float height = bounds.getHeight();

					float x = i * width;
					float y = 0;

					child->bounds.setValue({ x, y, width, height });
				}
			}
			else  // Vertical
			{
				for (int i = 0; i < childCount; i++)
				{
					LayoutComponent* child = children[i];

					float width = bounds.getWidth();
					float height = bounds.getHeight() / (float)childCount;

					float x = 0;
					float y = i * height;

					child->bounds.setValue({ x, y, width, height });
				}
			}
		}
		else // Don't stretch children
		{
			if (horizontal.getValue()) 
			{

				// Calculate total width of children first, for alignment
				float totalSpacingX = childCount - 1 * spacingX.getValue();
				float childrenWidth = 0;
				for (int i = 0; i < childCount; i++)
					childrenWidth += children[i]->bounds.getValueRef().getHeight();


				// Start at offset (leftover space * alignment)
				float x = (bounds.getWidth() - childrenWidth) * alignmentX.getValue();

				// Layout children
				for (int i = 0; i < childCount; i++) 
				{
					LayoutComponent* child = children[i];
					Rect childBounds = child->bounds.getValue();

					float y = (bounds.getHeight() - child->bounds.getValueRef().getHeight()) * alignmentY.getValue();

					child->bounds.setValue({ x, y, childBounds.getWidth(), childBounds.getHeight() });
					x += childBounds.getWidth() + spacingX.getValue();
				}
			}
			else // Vertical
			{ 

				// Calculate total height of children first, for alignment
				float totalSpacingY = childCount - 1 * spacingY.getValue();
				float childrenHeight = 0;
				for (int i = 0; i < childCount; i++)
					childrenHeight += children[i]->bounds.getValueRef().getHeight();

				// Start at offset (leftover space * alignment)
				float y = (bounds.getHeight() - childrenHeight) * alignmentY.getValue();

				// Layout children
				for (int i = 0; i < childCount; i++) 
				{
					LayoutComponent* child = children[i];
					Rect childBounds = child->bounds.getValue();

					float x = (bounds.getWidth() - child->bounds.getValueRef().getWidth()) * alignmentX.getValue();

					child->bounds.setValue({ x, y, childBounds.getWidth(), childBounds.getHeight() });
					y += childBounds.getHeight() + spacingY.getValue();
				}
			}
		}
		return true;
	}
}