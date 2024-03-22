/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "emptymesh.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmptyMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// NoMesh
	//////////////////////////////////////////////////////////////////////////

	EmptyMesh::EmptyMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool EmptyMesh::init(utility::ErrorState& errorState)
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
