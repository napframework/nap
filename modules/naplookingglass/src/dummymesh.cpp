/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "dummymesh.h"

// External includes
#include <nap/core.h>
#include <renderservice.h>

namespace nap
{
	DummyMesh::DummyMesh(Core& core) : mRenderService(core.getService<RenderService>()) {}

	bool DummyMesh::init(utility::ErrorState& errorState)
	{
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		mMeshInstance->setNumVertices(0);
		mMeshInstance->setUsage(EMeshDataUsage::Static);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(ECullMode::None);

		// Initialize the empty mesh instance
		return mMeshInstance->init(errorState);
	}
}
