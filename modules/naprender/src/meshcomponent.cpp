#include "meshcomponent.h"
#include "meshresource.h"

namespace nap
{
	// Draw Mesh
	void MeshComponent::onDraw()
	{
		opengl::Mesh* mesh = getMesh();
		if (mesh == nullptr)
		{
			return;
		}
		mesh->draw();
	}

	opengl::Mesh* MeshComponent::getMesh() const
	{
		return &mMesh->getMesh();
	}
}

RTTI_DEFINE(nap::MeshComponent)