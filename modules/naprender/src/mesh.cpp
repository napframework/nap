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
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::PositionVertexAttr("Position");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::NormalVertexAttr("Normal");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::UVVertexAttr("UV");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::ColorVertexAttr("Color");

	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::GetUVVertexAttr(int uvChannel)
	{
		std::ostringstream stream;
		stream << UVVertexAttr << uvChannel;
		return stream.str();
	}

	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::GetColorVertexAttr(int colorChannel)
	{
		std::ostringstream stream;
		stream << ColorVertexAttr << colorChannel;
		return stream.str();
	}


	Mesh::~Mesh()
	{
	}

	// Returns associated mesh
	opengl::GPUMesh& Mesh::getGPUMesh() const
	{
		assert(mGPUMesh != nullptr);
		return *mGPUMesh;
	}


	bool Mesh::init(utility::ErrorState& errorState)
	{
		if (mOwnedAttributes.empty())
		{
			mOwnedAttributes.reserve(mRTTIAttributes.size());
			for (auto& mesh_attribute : mRTTIAttributes)
			{
				std::unique_ptr<VertexAttribute> owned_mesh_attribute(mesh_attribute->get_type().create<VertexAttribute>());
				rtti::copyObject(*mesh_attribute, *owned_mesh_attribute);
				mOwnedAttributes.emplace_back(std::move(owned_mesh_attribute));
			}
		}

		mGPUMesh = std::make_unique<opengl::GPUMesh>();
		for (auto& mesh_attribute : mOwnedAttributes)
			mGPUMesh->addVertexAttribute(mesh_attribute->mAttributeID, mesh_attribute->getType(), mesh_attribute->getNumComponents(), mNumVertices, GL_STATIC_DRAW);

		update();

		return true;
	}

	void Mesh::update()
	{
		for (auto& mesh_attribute : mOwnedAttributes)
		{
			const opengl::VertexAttributeBuffer& vertex_attr_buffer = mGPUMesh->getVertexAttributeBuffer(mesh_attribute->mAttributeID);
			vertex_attr_buffer.setData(mesh_attribute->getData());
		}

		mGPUMesh->setIndices(mIndices);
	}

	void Mesh::moveFrom(Mesh& mesh)
	{
		mNumVertices		= mesh.mNumVertices;
		mDrawMode			= mesh.mDrawMode;
		mIndices			= mesh.mIndices;
		mGPUMesh			= std::move(mesh.mGPUMesh);
		mOwnedAttributes	= std::move(mesh.mOwnedAttributes);
	}

	//////////////////////////////////////////////////////////////////////////

	bool MeshFromFile::init(utility::ErrorState& errorState)
	{
		std::unique_ptr<Mesh> mesh = loadMesh(mPath, errorState);
		if (!errorState.check(mesh != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		moveFrom(*mesh);

		return true;
	}
}