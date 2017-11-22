#include "selectledmeshcomponent.h"

// External Includes
#include "entity.h"
#include <renderablemeshcomponent.h>
#include <mathutils.h>

// nap::selectledmeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectLedMeshComponent)
	RTTI_PROPERTY("Meshes", &nap::SelectLedMeshComponent::mMeshes, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TriangleComponent", &nap::SelectLedMeshComponent::mTriangleRenderableMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameComponent", &nap::SelectLedMeshComponent::mFrameRenderableMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index", &nap::SelectLedMeshComponent::mIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::selectledmeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectLedMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectLedMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}



	bool SelectLedMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy our meshes
		for (auto& mesh : getComponent<SelectLedMeshComponent>()->mMeshes)
			mLedMeshes.emplace_back(mesh.get());

		// Create renderable meshes, ie: bind a mesh to a material with an associated vao
		for (auto& mesh : mLedMeshes)
		{
			mDrawableTriangleMeshes.emplace_back(mTriangleRenderableMeshComponent->createRenderableMesh(*(mesh->mTriangleMesh), errorState));
			if (!mDrawableTriangleMeshes.back().isValid())
				return false;

			mDrawableFrameMeshes.emplace_back(mFrameRenderableMeshComponent->createRenderableMesh(*(mesh->mFrameMesh), errorState));
			if (!mDrawableFrameMeshes.back().isValid())
				return false;
		}

		// Select mesh to draw
		select(getComponent<SelectLedMeshComponent>()->mIndex);

		return true;
	}


	void SelectLedMeshComponentInstance::update(double deltaTime)
	{

	}



	void SelectLedMeshComponentInstance::select(int index)
	{
		assert(mLedMeshes.size() > 0);
		mIndex = math::clamp<int>(index, 0, mLedMeshes.size()-1);
		mTriangleRenderableMeshComponent->setMesh(mDrawableTriangleMeshes[mIndex]);
		mFrameRenderableMeshComponent->setMesh(mDrawableFrameMeshes[mIndex]);
	}

}