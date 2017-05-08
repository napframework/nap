// Local Includes
#include "rendercomponent.h"

namespace nap
{
	// Bind and draw
	void RenderableComponent::draw()
	{
		Material* mat = getMaterial();

		// Fetch material, should always be present
		if (mat == nullptr)
		{
			nap::Logger::warn("unable to resolve material for render-able component: %s", this->getName().c_str());
			return;
		}
	
		// Bind material
		mat->bind();

		// Upload uniforms
		mat->pushUniforms();

		// Draw
		onDraw();

		// Unbind material
		mat->unbind();
		onPostDraw();
	}

}

RTTI_DEFINE_BASE(nap::RenderableComponent)
