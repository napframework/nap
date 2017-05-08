#pragma once

// Local Includes
#include "rendercomponent.h"

// External Includes
#include <nmesh.h>

namespace nap
{
	class ModelResource;

	/**
	 * Represents a drawable mesh that can be used as a component in an object tree
	 * Every mesh points to a loaded mesh resource, this mesh owns the actual mesh data
	 * Every mesh is associated with a material instance that uses it's shader program 
	 * to draw itself to screen. The material instance is always a child of this component
	 */
	class MeshComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
	public:
		MeshComponent() = default;

		/**
		 * Draws the mesh associated with this object
		 * Doesn't draw anything if getMesh returns nullptr
		 */
		virtual void onDraw() override;

		/**
		 * Get the material used to draw this object
		 *
		 * @return material, nullptr if not found
		 */
		virtual Material* getMaterial() override;

	public:
		ModelResource* mModelResource = nullptr;
	};
}
