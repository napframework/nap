#include <napofupdatecomponent.h>

namespace nap
{
	void OFUpdatableComponent::update()
	{
		if (mEnableUpdates.getValue())
			onUpdate();
	}
}

RTTI_DEFINE(nap::OFUpdatableComponent)