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
		* Renders the model from the ModelResource, using the material on the ModelResource.
		*/
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		* @return The ModelResource associated with this component.
		*/
		ModelResource* getModelResource() { return mModelResource; }

	public:
		ModelResource* mModelResource = nullptr;
	};
}
