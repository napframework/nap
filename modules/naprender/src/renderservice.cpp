// Local Includes
#include "renderservice.h"
#include "rendercomponent.h"

// External Includes
#include <nap/core.h>

namespace nap
{	
	// Emits the draw call
	void RenderService::render()
	{
		draw.trigger();
	}


	// Register all types 
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
	}

} // Renderservice

RTTI_DEFINE(nap::RenderService)