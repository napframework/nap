#pragma once

// Local Includes
#include "rendercomponent.h"

namespace nap
{
	/**
	 * Represents a drawable mesh that can be used as a component in an object tree
	 * Every mesh points to a loaded mesh resource, this mesh owns the actual mesh data
	 * Every mesh is associated with a material instance that uses it's shader program 
	 * to draw itself to screen. The material instance is always a child of this component
	 */
	class MeshComponent : public RenderableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(RenderableComponent)
	public:
		MeshComponent() = default;

		/**
		 * Draws the mesh to screen
		 */
		virtual void onDraw() override		{ assert(false); }
	};
}

RTTI_DECLARE(nap::MeshComponent)
