#include "meshcomponent.h"

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
}

RTTI_DEFINE_BASE(nap::MeshComponent)