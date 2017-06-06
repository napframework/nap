// Local Includes
#include "meshresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"

RTTI_BEGIN_BASE_CLASS(nap::MeshResource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MeshFromFileResource)
	RTTI_PROPERTY("Path", &nap::MeshFromFileResource::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Returns associated mesh
	opengl::Mesh& MeshResource::getMesh() const
	{
		assert(mMesh != nullptr);
		return *mMesh;
	}

	//////////////////////////////////////////////////////////////////////////

	bool MeshFromFileResource::init(utility::ErrorState& errorState)
	{
		mMesh = loadMesh(mPath, errorState);
		if (!errorState.check(mMesh != nullptr, "Unable to load mesh %s", mPath.c_str()))
			return false;

		return true;
	}
}