// Local Includes
#include "mesh.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Mesh)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MeshFromFile)
	RTTI_PROPERTY("Path", &nap::MeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Returns associated mesh
	opengl::Mesh& Mesh::getMesh() const
	{
		assert(mMesh != nullptr);
		return *mMesh;
	}

	//////////////////////////////////////////////////////////////////////////

	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		mMesh = loadMesh(mPath, errorState);
		if (!errorState.check(mMesh != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		return true;
	}
}