#include "scatterpointsmesh.h"

// nap::scatterpointsmesh run time class definition 
RTTI_BEGIN_CLASS(nap::ScatterPointsMesh)
	RTTI_PROPERTY("ReferenceMesh", &nap::ScatterPointsMesh::mReferenceMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointCount", &nap::ScatterPointsMesh::mNumberOfPoints, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool ScatterPointsMesh::init(utility::ErrorState& errorState)
	{
		// make sure we have at least 1 point to scatter
		if (!errorState.check(mNumberOfPoints >= 0, 
			"invalid number of points: %s, need at least 1 point for scatter operation", this->mID.c_str()))
			return false;

		// create instance
		if (!createMeshInstance(errorState))
			return false;

		return true;
	}


	bool ScatterPointsMesh::createMeshInstance(nap::utility::ErrorState& error)
	{
		if (!error.check(mMeshInstance == nullptr, "unable to create new mesh, already assigned: %s", this->mID.c_str()))
			return false;

		// There is only 1 shape associated with the scatter mesh
		mMeshInstance = std::make_unique<MeshInstance>();
		mMeshInstance->createShape();
		return true;
	}


	bool ScatterPointsMesh::setup(nap::utility::ErrorState& error)
	{
		assert(mMeshInstance != nullptr);
		mMeshInstance->setNumVertices(mNumberOfPoints);
		return true;

	}

}