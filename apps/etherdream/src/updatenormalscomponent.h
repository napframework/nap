#pragma once

// External include
#include <component.h>
#include <visualizenormalsmesh.h>

namespace nap
{
	class UpdateNormalsComponentInstance;

	/**
	 *	Updates a normals mesh on update
	 */
	class UpdateNormalsComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateNormalsComponent, UpdateNormalsComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: link to the mesh that contains the normals
		rtti::ObjectPtr<nap::VisualizeNormalsMesh> mNormalMesh = nullptr;
	};


	/**
	 * updatenormalscomponentInstance	
	 */
	class UpdateNormalsComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateNormalsComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize updatenormalscomponentInstance based on the updatenormalscomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the updatenormalscomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Set the mesh to use as visualization
		 */
		void setMesh(nap::VisualizeNormalsMesh& mesh)					{ mNormalMesh = &mesh; }

		/**
		 *	Updates the mesh, CPU and GPU
		 */
		virtual void update(double deltaTime) override;

	private:
		nap::VisualizeNormalsMesh* mNormalMesh = nullptr;
	};
}
