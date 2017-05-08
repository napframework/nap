#include "meshcomponent.h"
#include "meshresource.h"
#include "modelresource.h"

namespace nap
{
	// Draw Mesh
	void MeshComponent::onDraw()
	{
		if (mModelResource == nullptr)
			return;

		mModelResource->draw();
	}

	Material* MeshComponent::getMaterial()
	{
		if (mModelResource == nullptr)
			return nullptr;

		return mModelResource->mMaterialResource;
	}
}

RTTI_DEFINE(nap::MeshComponent)