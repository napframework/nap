// Local Includes
#include "mesh.h"
#include <rtti/rttiutilities.h>

RTTI_BEGIN_CLASS(nap::RTTIMeshProperties)
	RTTI_PROPERTY("NumVertices",	&nap::RTTIMeshProperties::mNumVertices,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DrawMode",		&nap::RTTIMeshProperties::mDrawMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attributes",		&nap::RTTIMeshProperties::mAttributes,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Indices",		&nap::RTTIMeshProperties::mIndices,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS(nap::Mesh)
	RTTI_PROPERTY("Properties",		&nap::Mesh::mProperties,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMesh)
RTTI_END_CLASS


namespace nap
{
	const MeshInstance::VertexAttributeID MeshInstance::VertexAttributeIDs::GetPositionName() { return "Position"; }
	const MeshInstance::VertexAttributeID MeshInstance::VertexAttributeIDs::getNormalName()	{ return "Normal"; }

	const MeshInstance::VertexAttributeID MeshInstance::VertexAttributeIDs::GetUVName(int uvChannel)
	{
		std::ostringstream stream;
		stream << "UV" << uvChannel;
		return stream.str();
	}


	const MeshInstance::VertexAttributeID MeshInstance::VertexAttributeIDs::GetColorName(int colorChannel)
	{
		std::ostringstream stream;
		stream << "Color" << colorChannel;
		return stream.str();
	}


	MeshInstance::~MeshInstance()
	{
	}


	// Returns associated mesh
	opengl::GPUMesh& MeshInstance::getGPUMesh() const
	{
		assert(mGPUMesh != nullptr);
		return *mGPUMesh;
	}


	// Creates GPU vertex attributes and updates mesh
	bool MeshInstance::initGPUData(utility::ErrorState& errorState)
	{
		mGPUMesh = std::make_unique<opengl::GPUMesh>();
		for (auto& mesh_attribute : mProperties.mAttributes)
			mGPUMesh->addVertexAttribute(mesh_attribute->mAttributeID, mesh_attribute->getType(), mesh_attribute->getNumComponents(), mProperties.mNumVertices, GL_STATIC_DRAW);

		return update(errorState);
	}


	bool MeshInstance::init(utility::ErrorState& errorState)
	{
		return initGPUData(errorState);
	}


	bool MeshInstance::init(RTTIMeshProperties& meshProperties, utility::ErrorState& errorState)
	{
		mProperties.mAttributes.reserve(meshProperties.mAttributes.size());
		for (auto& mesh_attribute : meshProperties.mAttributes)
		{
			std::unique_ptr<BaseVertexAttribute> owned_mesh_attribute(mesh_attribute->get_type().create<BaseVertexAttribute>());
			rtti::copyObject(*mesh_attribute, *owned_mesh_attribute);
			mProperties.mAttributes.emplace_back(std::move(owned_mesh_attribute));
		}
		mProperties.mNumVertices = meshProperties.mNumVertices;
		mProperties.mDrawMode = meshProperties.mDrawMode;
		mProperties.mIndices = meshProperties.mIndices;

		return initGPUData(errorState);
	}


	bool MeshInstance::update(utility::ErrorState& errorState)
	{
		// Check for mismatches in sizes
		for (auto& mesh_attribute : mProperties.mAttributes)
		{
			if (!errorState.check(mesh_attribute->getCount() == mProperties.mNumVertices,
					"Vertex attribute %s has a different amount of elements (%d) than the mesh (%d)", mesh_attribute->mAttributeID.c_str(), mesh_attribute->getCount(), mProperties.mNumVertices))
			{
				return false;
			}
		}

		for (auto& mesh_attribute : mProperties.mAttributes)
		{
			const opengl::VertexAttributeBuffer& vertex_attr_buffer = mGPUMesh->getVertexAttributeBuffer(mesh_attribute->mAttributeID);
			vertex_attr_buffer.setData(mesh_attribute->getRawData());
		}

		if (!mProperties.mIndices.empty())
			mGPUMesh->getOrCreateIndexBuffer().setData(mProperties.mIndices);

		return true;
	}


	bool Mesh::init(utility::ErrorState& errorState)
	{
		return mMeshInstance.init(mProperties, errorState);
	}
}