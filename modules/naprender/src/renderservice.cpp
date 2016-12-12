// Local Includes
#include "renderservice.h"
#include "rendercomponent.h"

// External Includes
#include <nap/core.h>

namespace nap
{	
	// Register all types 
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
	}


	// Emits the draw call
	void RenderService::render()
	{
		// Get all render components
		std::vector<nap::RenderableComponent*> render_comps;
		getObjects<nap::RenderableComponent>(render_comps);

		// Draw
		for (auto& comp : render_comps)
			comp->draw();

		// Trigger
		draw.trigger();
	}

} // Renderservice

RTTI_DEFINE(nap::RenderService)