#pragma once

// Local Includes
#include "rendercomponent.h"

// External Includes
#include <nmesh.h>

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
		RTTI_ENABLE(RenderableComponent)
	public:
		MeshComponent() = default;

		/**
		 * Draws the mesh associated with this object
		 * Doesn't draw anything if getMesh returns nullptr
		 */
		virtual void onDraw() override;

		/**
		 * If this object currently holds a mesh
		 */
		bool hasMesh() const							{ return getMesh() != nullptr; }

		/**
		 * @return the mesh this component manages
		 * nullptr if mesh is invalid
		 */
		virtual opengl::Mesh* getMesh() const = 0;
	};
}
