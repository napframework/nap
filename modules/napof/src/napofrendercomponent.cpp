#include "napofrendercomponent.h"

namespace nap
{
	void OFRenderableComponent::draw()
	{
		if (mEnableDrawing.getValue())
			onDraw();
	}
}

RTTI_DEFINE(nap::OFRenderableComponent)