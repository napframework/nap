#include "ledmesh.h"

// nap::ledmesh run time class definition 
RTTI_BEGIN_CLASS(nap::LedMesh)
	RTTI_PROPERTY("TriangleMesh", &nap::LedMesh::mTriangleMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameMesh", &nap::LedMesh::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
	// Put additional properties here
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	LedMesh::~LedMesh()			{ }


	bool LedMesh::init(utility::ErrorState& errorState)
	{
		return true;
	}
}