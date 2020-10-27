/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External include
#include <component.h>
#include <visualizenormalsmesh.h>
#include <nap/resourceptr.h>

namespace nap
{
	class UpdateNormalsComponentInstance;

	/**
	 *	Updates a normals mesh on update
	 */
	class NAPAPI UpdateNormalsComponent : public Component
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
		ResourcePtr<VisualizeNormalsMesh> mNormalMesh = nullptr;
	};


	/**
	 * Pushes the updated mesh (which represents the normals of the line) to the GPU every frame
	 */
	class NAPAPI UpdateNormalsComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateNormalsComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initializes this instance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Set the mesh to use as visualization
		 */
		void setMesh(nap::VisualizeNormalsMesh& mesh)					{ mNormalMesh = &mesh; }

		/**
		 * Forces an update of the mesh that represents the normals
		 */
		virtual void update(double deltaTime) override;

	private:
		nap::VisualizeNormalsMesh* mNormalMesh = nullptr;
	};
}
