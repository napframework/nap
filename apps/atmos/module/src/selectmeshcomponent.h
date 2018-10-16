#pragma once

// local includes
#include "meshfromfile.h"

// external includes
#include <component.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <visualizenormalsmesh.h>

namespace nap
{
	class SelectMeshComponentInstance;

	/**
	 * Selects a mesh based on an index
	 * This is the resource that can be declared in JSON.
	 * This resource holds a list of meshes that the instance can switch between
	 */
	class NAPAPI SelectMeshComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectMeshComponent, SelectMeshComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		std::vector<ResourcePtr<IMesh>>				mMeshes;				///< Property: "Meshes" link to videos
		nap::ComponentPtr<RenderableMeshComponent>	mScanMeshComponent;		///< Property: "ScanMeshComponent" link to the component that renders the scanned mesh
		nap::ComponentPtr<RenderableMeshComponent>	mNormalsMeshComponent;	///< Property: "NormalsMeshComponent" link to the component that renders the normals mesh
		int mIndex = 0;														///< Property: "Index" current video index
	};


	/**
	* Instance (runtime version) of the mesh selector
	* Makes sure there is at least 1 available mesh and sets
	* that mesh when the selection changes on to the render able mesh component
	*/
	class NAPAPI SelectMeshComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectMeshComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectvideomeshcomponentInstance based on the selectvideomeshcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectvideomeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update selectvideomeshcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
	
		/**
		 * Selects a new mesh
		 * @param index new mesh index, clamped to mesh range
		 */
		void selectMesh(int index);

		/**
		 * @return the number of selectable meshes
		 */
		int getCount() const											{ return mMeshes.size(); }

		/**
		 * @return current mesh index
		 */
		int getIndex() const											{ return mCurrentIndex; }

		/**
		 * @return the normals mesh, nullptr if not found
		 */
		VisualizeNormalsMesh* getNormalsMesh();

	private:
		std::vector<RenderableMesh> mMeshes;									//< All meshes to select from
		int mCurrentIndex = 0;													//< Current video index
		RenderableMesh* mCurrentMesh = nullptr;									//< Current selected mesh
		VisualizeNormalsMesh* mNormalsMesh = nullptr;							//< Mesh that visualizes the normals
		ComponentInstancePtr<RenderableMeshComponent> mScanMeshComponent =		{ this, &SelectMeshComponent::mScanMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent> mNormalsMeshComponent =	{ this, &SelectMeshComponent::mNormalsMeshComponent };
	};
}
