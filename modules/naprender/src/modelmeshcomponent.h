#pragma once

// Internal Includes
#include "rendercomponent.h"
#include "modelresource.h"

// External Includes
#include <nap/resourcelinkattribute.h>

namespace nap
{
	/**
	 * Render's a mesh that resides inside a 3d model resource to screen or buffer
	 * The component links to a 3d model that holds all the vertex data in the form of meshes
	 * Use the meshIndex attribute to change what part of the 3d model this component references (works with)
	 * Every 3d model consists out of various ModelMeshComponents
	 */
	class ModelMeshComponent : public RenderableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(RenderableComponent)
	public:
		// Constructor
		ModelMeshComponent() = default;

		/**
		 * Selects which part of the model this component references
		 * When changing the index a different part of the model will be used
		 * when rendering the mesh to screen / buffer
		 */
		Attribute<int> meshIndex = { this, "meshIndex", 0 };

		/**
		 * Binds the selected mesh and draws it using currently associated material
		 */
		virtual void onDraw() override;

		/**
		 * Link to the model this mesh references
		 * By default this link is empty and needs to be set
		 */
		ResourceLinkAttribute modelResource = { this, "model", RTTI_OF(ModelResource) };
	};
}

RTTI_DECLARE(nap::ModelMeshComponent)
