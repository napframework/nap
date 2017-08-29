// Local Includes
#include "vertexattribute.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexAttribute)
	RTTI_PROPERTY("AttributeID", &nap::VertexAttribute::mAttributeID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FloatMeshAttribute)
	RTTI_PROPERTY("Data", &nap::FloatMeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::IntMeshAttribute)
	RTTI_PROPERTY("Data", &nap::IntMeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ByteMeshAttribute)
	RTTI_PROPERTY("Data", &nap::ByteMeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DoubleMeshAttribute)
	RTTI_PROPERTY("Data", &nap::DoubleMeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec2MeshAttribute)
	RTTI_PROPERTY("Data", &nap::Vec2MeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec3MeshAttribute)
	RTTI_PROPERTY("Data", &nap::Vec3MeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec4MeshAttribute)
	RTTI_PROPERTY("Data", &nap::Vec4MeshAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Only here to make sure this cpp is not removed during optimization, which would cause the RTTI definitions to be missingo
	VertexAttribute::VertexAttribute()
	{
	}
}