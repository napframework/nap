#pragma once

// Local Includes
#include "rendercomponent.h"
#include "material.h"
#include "nap/objectptr.h"

// External Includes
#include <nmesh.h>


namespace nap
{
	class RenderableMeshResource;

	/**
	 * Represents a drawable mesh that can be used as a component in an object tree
	 * Every RenderableMeshComponent has a pointer to a RenderableMeshResource, which
	 * is an object that represents a mesh with a material assigned to it. To override
	 * uniform shader values from that material for this particular instance, the 
	 * component has a MaterialInstance.
	 */
	class RenderableMeshComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)

	public:
		RenderableMeshComponent() = default;

		/**
		 * Tests whether the MaterialInstance is overriding uniforms from the mesh that we are drawing.
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * @return The ModelResource associated with this component.
		 */
		RenderableMeshResource* getRenderableMeshResource();

		/**
		 * @return MaterialInstance for this component.
		 */
		MaterialInstance* getMaterialInstance();

	private:
		void pushUniforms();

	public:
		ObjectPtr<RenderableMeshResource>	mRenderableMeshResource;			///< The mesh+material object
		ObjectPtr<MaterialInstance>			mMaterialInstance;					///< MaterialInstance, which is used to override uniforms for this instance
	};
}
