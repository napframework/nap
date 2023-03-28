#include "nomesh.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::NoMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// NoMesh
	//////////////////////////////////////////////////////////////////////////

	NoMesh::NoMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool NoMesh::init(utility::ErrorState& errorState)
	{
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		mMeshInstance->setNumVertices(0);
		mMeshInstance->setUsage(EMemoryUsage::Static);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(ECullMode::None);

		// Initialize no mesh instance
		return mMeshInstance->init(errorState);
	}
}
