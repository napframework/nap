// Local Includes
#include "meshresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"


RTTI_BEGIN_CLASS(nap::MeshResource)
	RTTI_PROPERTY_FILE_LINK("Path", &nap::MeshResource::mPath)
RTTI_END_CLASS

namespace nap
{
	// Returns associated mesh
	opengl::Mesh& MeshResource::getMesh() const
	{
		assert(mMesh != nullptr);
		return *mMesh;
	}

	bool MeshResource::init(InitResult& initResult)
	{
		mPrevMesh = std::move(mMesh);

		mMesh = loadMesh(mPath, initResult);
		if (!initResult.check(mMesh != nullptr, "Unable to load mesh %s", mPath.c_str()))
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

	bool CustomMeshResource::init(InitResult& initResult)
	{
		mPrevMesh = std::move(mMesh);
		
		mMesh = std::move(mCustomMesh);
		if (!initResult.check(mMesh != nullptr, "Unable to init custom mesh"))
			return false;

		return true;
	}
}

RTTI_DEFINE(nap::MeshResource)