
#include "napoflayoutplanecomponent.h"

RTTI_DEFINE(nap::OFLayoutPlaneComponent)

namespace nap {

	OFLayoutPlaneComponent::OFLayoutPlaneComponent() : OFPlaneComponent()
	{
		mLayoutComponent.added.connect([&](LayoutComponent& layout) {
			layout.layoutChanged.connect(layoutSlot);
		});
		mLayoutComponent.removed.connect([&](LayoutComponent& layout) {
			layout.layoutChanged.disconnect(layoutSlot);
		});
	}
	
	void OFLayoutPlaneComponent::onPostDraw()
	{
		OFPlaneComponent::onPostDraw();

		if (debugDraw.getValueRef()) 
		{
			ofSetColor(0xFF, 0x00, 0xFF, 0xFF);
			ofNoFill();
			ofVec3f boundsMin;
			ofVec3f boundsMax;
			getBounds(boundsMin, boundsMax);

			ofDrawRectangle(
				boundsMin.x,
				boundsMin.y,
				boundsMax.x - boundsMin.x,
				boundsMax.y - boundsMin.y
				);

			if (mLayoutComponent) 
			{
				Margins margins = mLayoutComponent->margins.getValue();

				// Convert margins from cm to pixels
				margins.setLeft(margins.getLeft() * mLayoutComponent->ppcmX.getValue());
				margins.setTop(margins.getTop() * mLayoutComponent->ppcmY.getValue());
				margins.setRight(margins.getRight() * mLayoutComponent->ppcmX.getValue());
				margins.setBottom(margins.getBottom() * mLayoutComponent->ppcmY.getValue());

				ofSetColor(0xFF, 0x00, 0xFF, 0x88);
				ofDrawRectangle(
					boundsMin.x + margins.getLeft(),
					boundsMin.y + margins.getBottom(),
					boundsMax.x - boundsMin.x - margins.getRight() - margins.getLeft(),
					boundsMax.y - boundsMin.y - margins.getBottom() - margins.getTop()
					);
				ofDrawBitmapString(mLayoutComponent->getName(), boundsMin.x + 2, boundsMin.y + 3, boundsMin.z);
			}

			float crossSize = 50;
			ofSetColor(0, 255, 255);
			ofLine(-crossSize, 0, 0, crossSize, 0, 0);
			ofLine(0, -crossSize, 0, 0, crossSize, 0);
		}
	}

	// Based on the provided layout metrics, 
	// resize and move the centered quad as if it's a top-left aligned view.
	void OFLayoutPlaneComponent::onLayout(LayoutComponent& layout)
	{
		if (!mTransform) 
		{
			Logger::fatal("Failed to find transform component: '%s'", getName().c_str());
			return;
		}
		if (!mLayoutComponent) 
		{
			Logger::fatal("Failed to find layout component: '%s'", getName().c_str());
			return;
		}
		
		Rect rect = mLayoutComponent->getBounds();

		// Pixels per centimeter
		ofVec3f ppcm(mLayoutComponent->ppcmX.getValue(), mLayoutComponent->ppcmY.getValue(), 0);
		ppcm.z = (ppcm.x + ppcm.y) / 2.0f;

		// Set plane size
		float w = rect.getWidth()  * ppcm.x;
		float h = rect.getHeight() * ppcm.y;

		// Only update width / height if different (call has a lot of overhead)
		if(w != getWidth())
			setWidth(w);

		if(h != getHeight())
			setHeight(h);

		// Calculate transform
		float x = rect.getX() + rect.getWidth() / 2;
		float y = rect.getY() + rect.getHeight() / 2;

		// Apply xform
		LayoutComponent* parentLayout = mLayoutComponent->getParentLayout();
		if (parentLayout) 
		{
			const auto& parentRect = parentLayout->getBounds();
			const auto& parentMgin = parentLayout->getMargins();
			x -= parentRect.getWidth() / 2 - parentMgin.getLeft();
			y -= parentRect.getHeight() / 2 - parentMgin.getTop();
		}


		mTransform->mTranslate.setValue(ofVec3f(x, -y, layout.getDepth()) * ppcm);
	}

	void OFLayoutPlaneComponent::onDebugDrawChanged(bool enabled)
	{
		// Propagate to children
		Entity* parent = getParent();
		if (!parent)
			return;

		for (OFLayoutPlaneComponent* layout : parent->getChildrenOfType<OFLayoutPlaneComponent>())
			layout->debugDraw.setValue(enabled);
	}

    Point OFLayoutPlaneComponent::layoutToWorld(const Point &p) {
        ofVec3f ppcm(mLayoutComponent->ppcmX.getValue(), mLayoutComponent->ppcmY.getValue(), 0);
        ppcm.z = (ppcm.x + ppcm.y) / 2.0f;


        Point out;
        out.setX(p.getX() * ppcm.x);
        out.setY(p.getY() * ppcm.y);

        LayoutComponent* parentLayout = mLayoutComponent->getParentLayout();
        if (parentLayout) {
            const auto& parentRect = parentLayout->getBounds();
            const auto& parentMgin = parentLayout->getMargins();
            out.setX(out.getX() - (parentRect.getWidth() / 2-parentMgin.getLeft()));
            out.setY(out.getY() - (parentRect.getHeight() / 2-parentMgin.getTop()));
        }
        out.setY(-out.getY());

        return out;
    }

}