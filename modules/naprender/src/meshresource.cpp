// Local Includes
#include "meshresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"


RTTI_BEGIN_CLASS(nap::MeshResource)
	RTTI_PROPERTY("Path", &nap::MeshResource::mPath, rtti::EPropertyMetaData::FileLink | rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Returns associated mesh
	opengl::Mesh& MeshResource::getMesh() const
	{
		assert(mMesh != nullptr);
		return *mMesh;
	}

	bool MeshResource::init(utility::ErrorState& errorState)
	{
		mPrevMesh = std::move(mMesh);

		mMesh = loadMesh(mPath, errorState);
		if (!errorState.check(mMesh != nullptr, "Unable to load mesh %s", mPath.c_str()))
			return false;

		return true;
	}

	void MeshResource::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			mPrevMesh = nullptr;
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			mMesh = std::move(mPrevMesh);
		}
	}

	const std::string MeshResource::getDisplayName() const
	{
		return getFileNameWithoutExtension(mPath);
	}

	bool CustomMeshResource::init(utility::ErrorState& errorState)
	{
		mPrevMesh = std::move(mMesh);
		
		mMesh = std::move(mCustomMesh);
		if (!errorState.check(mMesh != nullptr, "Unable to init custom mesh"))
			return false;

		return true;
	}
}

RTTI_DEFINE(nap::MeshResource)