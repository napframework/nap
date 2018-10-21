#include "selectmeshcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <mathutils.h>
#include <visualizenormalsmesh.h>

// nap::selectvideomeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectMeshComponent)
	RTTI_PROPERTY("Meshes",					&nap::SelectMeshComponent::mMeshes,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ScanMeshComponent",		&nap::SelectMeshComponent::mScanMeshComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NormalsMeshComponent",	&nap::SelectMeshComponent::mNormalsMeshComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",	&nap::SelectMeshComponent::mIndex,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::selectvideomeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
	}


	bool SelectMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over meshes
		nap::SelectMeshComponent* resource = getComponent<SelectMeshComponent>();
		
		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		for (auto& mesh : resource->mMeshes)
		{
			RenderableMesh render_mesh = mScanMeshComponent->createRenderableMesh(*mesh, errorState);
			if (!render_mesh.isValid())
				return false;
			mMeshes.emplace_back(render_mesh);
		}

		// Make sure we have some videos
		if (!errorState.check(mMeshes.size() > 0, "No mesh files to select"))
			return false;

		// Make sure the mesh component that works with the normals is indeed a normals mesh
		if (!errorState.check(mNormalsMeshComponent->getMesh().get_type().is_derived_from(RTTI_OF(nap::VisualizeNormalsMesh)), "Component that visualizes the normals does not reference a VisualizeNormalsMesh"))
			return false;

		// Copy normals mesh
		mNormalsMesh = static_cast<nap::VisualizeNormalsMesh*>(&(mNormalsMeshComponent->getMesh()));

		// Select the mesh to display based on resource index
		selectMesh(resource->mIndex);

		return true;
	}


	void SelectMeshComponentInstance::update(double deltaTime)
	{

	}


	void SelectMeshComponentInstance::selectMesh(int index)
	{
		mCurrentIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mCurrentMesh = &mMeshes[mCurrentIndex];
		
		// Set the new mesh
		mScanMeshComponent->setMesh(*mCurrentMesh);

		VisualizeNormalsMesh* normals_mesh = getNormalsMesh();
		assert(normals_mesh != nullptr);
		
		nap::utility::ErrorState error;
		normals_mesh->setReferenceMesh(mCurrentMesh->getMesh(), error);

		normals_mesh->calculateNormals(error, true);
	}

	nap::VisualizeNormalsMesh* SelectMeshComponentInstance::getNormalsMesh()
	{
		if (!(mNormalsMeshComponent->getMesh().get_type().is_derived_from(RTTI_OF(nap::VisualizeNormalsMesh))))
			return nullptr;
		return static_cast<nap::VisualizeNormalsMesh*>(&(mNormalsMeshComponent->getMesh()));
	}

}