#include "videomeshfromfile.h"

// External includes
#include <fbxconverter.h>
#include <meshutils.h>
#include <triangleiterator.h>

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

		// Get position
		mPositionAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		assert(mPositionAttribute != nullptr);

		// Get uv
		mUVAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		if (!errorState.check(mUVAttribute != nullptr, "unable to find uv attribute: %s on mesh: %s", VertexAttributeIDs::getUVName(0).c_str(), mPath.c_str()))
			return false;

		// Now check for the color attribute
		mNormalAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		if (!errorState.check(mNormalAttribute != nullptr, "unable to find normal attribute: %s on mesh: %s", VertexAttributeIDs::getNormalName().c_str(), mPath.c_str()))
			return false;

		// We add the direction attribute and copy our normal information
		// The mesh normals contain the direction of displacement
		mDirectionAttribute = &mMeshInstance->getOrCreateAttribute<glm::vec3>("DisplacementDirection");
		std::vector<glm::vec3> tri_displ_data(mNormalAttribute->getCount(), { 0.0f, 0.0f, 0.0f });
		mDirectionAttribute->setData(tri_displ_data);

		// Also add central uv attribute, used to sample displacement
		mUVCenterAttribute = &mMeshInstance->getOrCreateAttribute<glm::vec3>("UVCenter");
		std::vector<glm::vec3> uv_center_data(mUVAttribute->getCount(), {0.0f,0.0f,0.0f});
		mUVCenterAttribute->setData(uv_center_data);

		TriangleIterator tri_iterator(*mMeshInstance);
		while (!tri_iterator.isDone())
		{
			Triangle triangle = tri_iterator.next();

			// Calculate & set average
			glm::vec3 uv_avg = { 0.0, 0.0, 0.0 };
			TriangleData<glm::vec3> uvTriangleData = triangle.getVertexData(*mUVAttribute);
			uv_avg += uvTriangleData.first();
			uv_avg += uvTriangleData.second();
			uv_avg += uvTriangleData.third();
			uv_avg /= 3.0f;
			
			triangle.setVertexData(*mUVCenterAttribute, uv_avg);
			
			// Calculate & set normal			
			glm::vec3 tri_normal = glm::normalize(computeTriangleNormal(triangle.indices(), *mPositionAttribute));
			triangle.setVertexData(*mDirectionAttribute, tri_normal);
		}

		// Initialize the mesh
		return errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str());
	}
}