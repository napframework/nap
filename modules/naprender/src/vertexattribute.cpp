// Local Includes
#include "vertexattribute.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexAttribute)
	RTTI_PROPERTY("AttributeID", &nap::VertexAttribute::mAttributeID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FloatVertexAttribute)
	RTTI_PROPERTY("Data", &nap::FloatVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::IntVertexAttribute)
	RTTI_PROPERTY("Data", &nap::IntVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ByteVertexAttribute)
	RTTI_PROPERTY("Data", &nap::ByteVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DoubleVertexAttribute)
	RTTI_PROPERTY("Data", &nap::DoubleVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec2VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec2VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec3VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec3VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec4VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec4VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Only here to make sure this cpp is not removed during optimization, which would cause the RTTI definitions to be missingo
	VertexAttribute::VertexAttribute()
	{
	}

	//////////////////////////////////////////////////////////////////////////

	template<>
	GLenum FloatVertexAttribute::getType() const { return GL_FLOAT; }

	template<>
	int FloatVertexAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum IntVertexAttribute::getType() const { return GL_INT; }

	template<>
	int IntVertexAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum ByteVertexAttribute::getType() const { return GL_BYTE; }

	template<>
	int ByteVertexAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum DoubleVertexAttribute::getType() const { return GL_DOUBLE; }

	template<>
	int DoubleVertexAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum Vec2VertexAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec2VertexAttribute::getNumComponents() const { return 2; }

	template<>
	GLenum Vec3VertexAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec3VertexAttribute::getNumComponents() const { return 3; }

	template<>
	GLenum Vec4VertexAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec4VertexAttribute::getNumComponents() const { return 4; }
}