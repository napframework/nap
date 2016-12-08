// Local Includes
#include "rendercomponent.h"

namespace nap
{
	// On construction adds a material instance
	// Material can't be removed
	RenderableComponent::RenderableComponent()
	{
		mMaterial = &addChild<Material>("material");
		mMaterial->setFlag(nap::ObjectFlag::Removable, false);
	}

	// Bind and draw
	void RenderableComponent::draw()
	{
		assert(false);
		onDraw();
		onPostDraw();
	}
}