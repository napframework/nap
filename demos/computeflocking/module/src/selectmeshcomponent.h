/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <renderablemeshcomponent.h>
#include <parameternumeric.h>

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

		std::vector<ResourcePtr<IMesh>> mMeshes;				///< Property: "Meshes" link to meshes
		ResourcePtr<ParameterInt> mIndex;						///< Property: "Index" current index
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
		 * Initialize SelectMeshComponentInstance based on the SelectMeshComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the SelectMeshComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

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

	private:
		std::vector<RenderableMesh> mMeshes;							//< All meshes to select from
		int mCurrentIndex = 0;											//< Current mesh index

		RenderableMesh* mCurrentMesh = nullptr;							//< Current selected mesh
		RenderableMeshComponentInstance* mRenderComponent = nullptr;	//< Renderable mesh component that receives the new selection

		ParameterInt* mIndexParam = nullptr;
	};
}
