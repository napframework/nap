#pragma once

#include "ledmesh.h"

#include <nap/component.h>
#include <nap/componentptr.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class SelectLedMeshComponentInstance;

	/**
	 *	selectledmeshcomponent
	 */
	class SelectLedMeshComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectLedMeshComponent, SelectLedMeshComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Get all the meshes to select from
		std::vector<ObjectPtr<LedMesh>> mMeshes;

		// Renderable mesh component used for rendering one of the selected meshes
		ComponentPtr<RenderableMeshComponent> mTriangleRenderableMeshComponent;
		ComponentPtr<RenderableMeshComponent> mFrameRenderableMeshComponent;

		// property: mesh index to use
		int mIndex = 0;
	};


	/**
	 * selectledmeshcomponentInstance	
	 */
	class SelectLedMeshComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectLedMeshComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectledmeshcomponentInstance based on the selectledmeshcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectledmeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update selectledmeshcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Selects the current mesh to render
		 * @param index the mesh index to use
		 */
		void select(int index);

		/**
		 *	@return the total number of selectable led meshes
		 */
		int getCount() const																{ return mLedMeshes.size(); }

		/**
		 *	@return the current selection index
		 */
		int getIndex() const																{ return mIndex; }

		/**
		 *	@return all mesh compounds
		 */
		std::vector<LedMesh*>& getLedMeshes()												{ return mLedMeshes; }

		/**
		 * @return all drawable triangle meshes
		 */
		std::vector<nap::RenderableMesh>& getTriangleMeshes() 								{ return mDrawableTriangleMeshes; }

		/**
		 * @return all drawable frame meshes
		 */
		std::vector<nap::RenderableMesh>& getFrameMeshes()									{ return mDrawableFrameMeshes; }

	private:
		// Component ptrs
		ComponentInstancePtr<RenderableMeshComponent> mTriangleRenderableMeshComponent =	{ this, &nap::SelectLedMeshComponent::mTriangleRenderableMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent> mFrameRenderableMeshComponent =		{ this, &nap::SelectLedMeshComponent::mFrameRenderableMeshComponent };

		// All the meshes we are able to select between
		std::vector<LedMesh*> mLedMeshes;

		// All the renderable frame meshes
		std::vector<nap::RenderableMesh> mDrawableTriangleMeshes;
		std::vector<nap::RenderableMesh> mDrawableFrameMeshes;

		// Current selected mesh index
		int mIndex = 0;
	};
}
