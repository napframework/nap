// Local Includes
#include "meshfromfile.h"
#include "fbxconverter.h"

RTTI_BEGIN_CLASS(nap::MeshFromFile)
	RTTI_PROPERTY_FILELINK("Path", &nap::MeshFromFile::mPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Mesh)
RTTI_END_CLASS

namespace nap
{
	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Initialize the mesh
		if (!errorState.check(mesh_instance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Move
		mMeshInstance = std::move(mesh_instance);
		return true;
	}
}