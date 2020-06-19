// Local Includes
#include "meshfromfile.h"
#include "fbxconverter.h"
#include "renderservice.h"

// External Includes
#include <nap/logger.h>
#include "nap/core.h"

RTTI_BEGIN_CLASS(nap::MeshFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::MeshFromFile::mUsage,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY_FILELINK("Path",	&nap::MeshFromFile::mPath,		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Mesh)
RTTI_END_CLASS

namespace nap
{
	MeshFromFile::MeshFromFile() :
		mRenderService(nullptr)
	{
	}


	MeshFromFile::MeshFromFile(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}


	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		nap::Logger::info("loading mesh: %s", mPath.c_str());

		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(*mRenderService, mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Set the usage for the mesh
		mesh_instance->setUsage(mUsage);

		// Initialize the mesh
		if (!errorState.check(mesh_instance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Move
		mMeshInstance = std::move(mesh_instance);
		return true;
	}
}