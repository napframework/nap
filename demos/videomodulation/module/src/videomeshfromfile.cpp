#include "videomeshfromfile.h"

// External includes
#include <fbxconverter.h>
#include <meshutils.h>

// nap::videomesh run time class definition 
RTTI_BEGIN_CLASS(nap::VideoMeshFromFile)
	RTTI_PROPERTY("Path", &nap::VideoMeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	VideoMeshFromFile::~VideoMeshFromFile()			{ }


	bool VideoMeshFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Set mesh
		mMeshInstance = std::move(mesh_instance);

		// Now check for the color attribute
		mColorAttribute = mMeshInstance->findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
		if (!errorState.check(mColorAttribute != nullptr, "unable to find color attribute: %s on mesh: %s", VertexAttributeIDs::GetColorName(0).c_str(), mPath.c_str()))
			return false;

		// Get position
		mPositionAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		assert(mPositionAttribute != nullptr);

		// Get uv
		mUVAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		if (!errorState.check(mUVAttribute != nullptr, "unable to find uv attribute: %s on mesh: %s", VertexAttributeIDs::getUVName(0).c_str(), mPath.c_str()))
			return false;

		// Now check for the color attribute
		mNormalAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		if (!errorState.check(mColorAttribute != nullptr, "unable to find normal attribute: %s on mesh: %s", VertexAttributeIDs::getNormalName().c_str(), mPath.c_str()))
			return false;

		// We add the direction attribute and copy our normal information
		// The mesh normals contain the direction of displacement
		mDirectionAttribute = &mMeshInstance->getOrCreateAttribute<glm::vec3>("DisplacementDirection");
		mDirectionAttribute->setData(mNormalAttribute->getData());

		computeNormals(*mMeshInstance, *mPositionAttribute, *mNormalAttribute);

		// Initialize the mesh
		return errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str());
	}
}