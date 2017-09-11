// Local Includes
#include "meshfromfile.h"
#include "fbxconverter.h"

RTTI_BEGIN_CLASS(nap::MeshFromFile)
	RTTI_PROPERTY("Path", &nap::MeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		mMeshInstance = std::move(mesh_instance);
		return true;
	}
}