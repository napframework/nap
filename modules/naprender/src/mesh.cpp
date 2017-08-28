// Local Includes
#include "mesh.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"
#include "rtti/rttiutilities.h"

RTTI_BEGIN_CLASS(nap::Mesh)
	RTTI_PROPERTY("NumVertices",	&nap::Mesh::mNumVertices,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DrawMode",		&nap::Mesh::mDrawMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RTTIAttributes", &nap::Mesh::mRTTIAttributes,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Indices",		&nap::Mesh::mIndices,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS(nap::MeshFromFile)
	RTTI_PROPERTY("Path", &nap::MeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MeshAttribute)
	RTTI_PROPERTY("AttributeID", &nap::MeshAttribute::mAttributeID, nap::rtti::EPropertyMetaData::Required)
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
	Mesh::~Mesh()
	{
	}

	// Returns associated mesh
	opengl::Mesh& Mesh::getMesh() const
	{
		assert(mGPUMesh != nullptr);
		return *mGPUMesh;
	}


	bool Mesh::init(utility::ErrorState& errorState)
	{
		initMesh(mRTTIAttributes);
		return true;
	}


	void Mesh::initMesh(const RTTIAttributeList& rttiAttributes)
	{
		mOwnedAttributes.reserve(rttiAttributes.size());
		for (auto& mesh_attribute : rttiAttributes)
		{
			std::unique_ptr<MeshAttribute> owned_mesh_attribute(mesh_attribute->get_type().create<MeshAttribute>());
			rtti::copyObject(*mesh_attribute, *owned_mesh_attribute);
			mOwnedAttributes.emplace_back(std::move(owned_mesh_attribute));
		}

// 		for (auto& mesh_attribute : mOwnedAttributes)
// 		{
// 			mGPUMesh->addVertexAttribute(mesh_attribute->mAttributeID, , , mNumVertices, GL_STATIC_DRAW);
// 		}
		update();
	}

	void Mesh::update()
	{
// 		mGPUMesh->clear();
// 		for (auto& mesh_attribute : mOwnedAttributes)
// 		{
// 			mGPUMesh->addVertexAttribute<T>(mesh_attribute->mAttributeID, mesh_attribute->getData());
// 		}
	}


	//////////////////////////////////////////////////////////////////////////

	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		std::unique_ptr<Mesh> mesh = loadMesh(mPath, errorState);
		if (!errorState.check(mesh != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		mNumVertices	= mesh->mNumVertices;
		mDrawMode		= mesh->mDrawMode;
		mIndices		= mesh->mIndices;

		initMesh(mesh->mRTTIAttributes);
		return true;
	}
}