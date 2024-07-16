/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "emptymesh.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>

 RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmptyMesh, "A mesh without data for simple render operations, with shaders without geometry")
	RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Usage",			&nap::EmptyMesh::mUsage,		nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DrawMode",		&nap::EmptyMesh::mDrawMode,	    nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("CullMode",		&nap::EmptyMesh::mCullMode,	    nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("PolygonMode",	&nap::EmptyMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default)
 RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// NoMesh
	//////////////////////////////////////////////////////////////////////////

	EmptyMesh::EmptyMesh(Core& core) : mRenderService(core.getService<RenderService>())
	{ }


	bool EmptyMesh::init(utility::ErrorState& errorState)
	{
		// Initialize no mesh instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

        // Configure the mesh instance
        mMeshInstance->setUsage(mUsage);
        mMeshInstance->setDrawMode(mDrawMode);
        mMeshInstance->setCullMode(mCullMode);
        mMeshInstance->setPolygonMode(mPolygonMode);
        mMeshInstance->setNumVertices(0);

		return mMeshInstance->init(errorState);
	}
}
