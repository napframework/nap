/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "meshfromfile.h"
#include "fbxconverter.h"
#include "renderservice.h"

// External Includes
#include <nap/logger.h>
#include "nap/core.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MeshFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::MeshFromFile::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::MeshFromFile::mCullMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::MeshFromFile::mPolygonMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY_FILELINK("Path",	&nap::MeshFromFile::mPath,			nap::rtti::EPropertyMetaData::Required,		nap::rtti::EPropertyFileType::Mesh)
RTTI_END_CLASS

namespace nap
{
	MeshFromFile::MeshFromFile(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		nap::Logger::info("loading mesh: %s", mPath.c_str());
		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(*mRenderService, mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Set the usage and cull mode for the mesh
		mesh_instance->setUsage(mUsage);
		mesh_instance->setCullMode(mCullMode);
		mesh_instance->setDrawMode(EDrawMode::Triangles);
		mesh_instance->setPolygonMode(mPolygonMode);

		// Initialize the mesh
		if (!errorState.check(mesh_instance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Move
		mMeshInstance = std::move(mesh_instance);
		return true;
	}
}
