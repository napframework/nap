/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "selectvideomeshcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <mathutils.h>

// nap::selectvideomeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectVideoMeshComponent, "Video mesh selector")
	RTTI_PROPERTY("Meshes",	&nap::SelectVideoMeshComponent::mMeshes,	nap::rtti::EPropertyMetaData::Required,	"All available video mesh files")
	RTTI_PROPERTY("Index",	&nap::SelectVideoMeshComponent::mIndex,		nap::rtti::EPropertyMetaData::Default,	"Initial mesh selection")
RTTI_END_CLASS

// nap::selectvideomeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectVideoMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectVideoMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
	}


	bool SelectVideoMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over meshes
		nap::SelectVideoMeshComponent* resource = getComponent<SelectVideoMeshComponent>();

		// Get component we want to render the meshes with
		mRenderComponent = getEntityInstance()->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(mRenderComponent != nullptr, "%s: missing RenderableMeshComponent", mID.c_str()))
			return false;

		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		for (auto& mesh : resource->mMeshes)
		{
			RenderableMesh render_mesh = mRenderComponent->createRenderableMesh(*mesh, errorState);
			if (!render_mesh.isValid())
				return false;
			mMeshes.emplace_back(render_mesh);
		}

		// Make sure we have some videos
		if (!errorState.check(mMeshes.size() > 0, "No mesh files to select"))
			return false;

		// Select the mesh to display based on resource index
		selectMesh(resource->mIndex);

		return true;
	}


	void SelectVideoMeshComponentInstance::update(double deltaTime)
	{

	}


	void SelectVideoMeshComponentInstance::selectMesh(int index)
	{
		mCurrentIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mCurrentMesh = &mMeshes[mCurrentIndex];
		mRenderComponent->setMesh(*mCurrentMesh);
	}

}
