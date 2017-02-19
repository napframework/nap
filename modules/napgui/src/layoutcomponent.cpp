#include "layoutcomponent.h"
#include <assert.h>

RTTI_DEFINE(nap::LayoutComponent)

namespace nap
{

	const float LayoutComponent::mDepthIncrement = 0.1f;

	void LayoutComponent::getLayoutChildren(std::vector<LayoutComponent*>& outChildren)
	{
		if (!getParentObject()) // Root?
			return;
        
        // From all child entities, get layoutcomponents

		Entity* parent = getParent();

		for (Entity* child : parent->getChildrenOfType<Entity>()) {
			for (auto layout : child->getChildrenOfType<LayoutComponent>()) {
				outChildren.emplace_back(layout);
			}
		}
	}

	std::vector<LayoutComponent*> LayoutComponent::getLayoutChildren()
	{
		std::vector<LayoutComponent*> children;
		getLayoutChildren(children);
		return children;
	}

	Rect LayoutComponent::getContentRect() const
	{
		const Rect& bound = bounds.getValue();
		const Margins& margin = margins.getValue();

		// clang-format off
		return Rect(
			bound.getX() + margin.getLeft(),
			bound.getY() + margin.getTop(),
			bound.getWidth() - margin.getLeft() - margin.getRight(),
			bound.getHeight() - margin.getTop() - margin.getBottom()
			);
		// clang-format on
	}

	float LayoutComponent::getDepth()
	{
		auto parentLayout = getParentLayout();
		if (parentLayout) {
			return mDepthIncrement + mDepthOffset.getValue();
		}
		return mDepth;
	}


	float LayoutComponent::pixelsToCmX(float inPixel) 
	{ 
		Attribute<float>* target_attr = ppcmX.getTarget<Attribute<float>>();
		assert(target_attr != nullptr);
		return inPixel / target_attr->getValue(); 
	}


	float LayoutComponent::pixelsToCmY(float inPixel) 
	{ 
		Attribute<float>* target_attr = ppcmY.getTarget<Attribute<float>>();
		assert(target_attr != nullptr);
		return inPixel / target_attr->getValue();
	}


	// Recursive function that allows children to perform a layout operation
	// Before propagating it through
	void LayoutComponent::validateLayout()
	{
		// Perform update for current layout, if return value is true, update children
		if (layout()) {
			// Update all child layouts
			for (auto childLayout : getLayoutChildren()) {
				childLayout->validateLayout();
			}
			layoutChanged.trigger(*this);
		} else {
			// Forward only plane updates if the layout should not propagate down to children
			updateLayoutPlane();
		}
	}


	// Updates only the layout planes
	void LayoutComponent::updateLayoutPlane()
	{
		for (auto child : getLayoutChildren()) {
			child->updateLayoutPlane();
		}
		layoutChanged.trigger(*this);
	}


	LayoutComponent* LayoutComponent::getParentLayout()
	{
		auto ownerEntity = getParentObject();
		assert(ownerEntity);

		auto parentEntity = ownerEntity->getParentObject();
		if (!parentEntity) // Root?
			return nullptr;

		std::vector<LayoutComponent*> layoutComponents = parentEntity->getChildrenOfType<LayoutComponent>();
		if (layoutComponents.empty()) // No parent layout component?
			return nullptr;

		if (layoutComponents.size() > 1) {
			Logger::warn("Entity '%s' has more than one LayoutComponent", parentEntity->getName().c_str());
		}

		return layoutComponents[0];
	}

	Point LayoutComponent::mapFromParent(const Point& p)
	{
		auto parent = getParentLayout();
		if (!parent)
			return p;
		return p - parent->margins.getValue().getTopLeft() - parent->bounds.getValue().getTopLeft();
	}

	Point LayoutComponent::mapToParent(const Point& p)
	{
		auto parent = getParentLayout();
		return parent->bounds.getValue().getTopLeft() + parent->margins.getValue().getTopLeft() + p;
	}

	Point LayoutComponent::mapFromRoot(const Point& p) {
		std::list<LayoutComponent*> pathToRoot;
		LayoutComponent* parent = getParentLayout();

		pathToRoot.push_front(this);
		while(parent) {
			pathToRoot.push_front(parent);
			parent = parent->getParentLayout();
		}

		Point result = p;
		for (auto child : pathToRoot) {
			result = child->mapFromParent(result);
		}
		return result;
	}


	// Walk up to root layout while mapping point p into each parent's space
	Point LayoutComponent::mapToRoot(const Point& p)
	{
		Point result = p;
		LayoutComponent* parent = getParentLayout();
		while (parent) {
			result = mapToParent(result);
			parent = parent->getParentLayout();
		}
		return result;
	}

    Point LayoutComponent::mapFromLayout(LayoutComponent &sourceLayout, const Point &p) {
        Point rootPoint = sourceLayout.mapToRoot(p);
        return mapFromRoot(rootPoint);
    }
}
