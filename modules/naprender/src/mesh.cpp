// Local Includes
#include "mesh.h"
#include <rtti/rttiutilities.h>
#include "meshutils.h"

RTTI_BEGIN_CLASS(nap::MeshShape)
	RTTI_PROPERTY("DrawMode",		&nap::MeshShape::mDrawMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Indices",		&nap::MeshShape::mIndices,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RTTIMeshProperties)
	RTTI_PROPERTY("NumVertices",	&nap::RTTIMeshProperties::mNumVertices,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attributes",		&nap::RTTIMeshProperties::mAttributes,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shapes",			&nap::RTTIMeshProperties::mShapes,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS(nap::Mesh)
	RTTI_PROPERTY("Properties",		&nap::Mesh::mProperties,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMesh)
RTTI_END_CLASS


namespace nap
{
	const std::string VertexAttributeIDs::getPositionName() { return "Position"; }
	const std::string VertexAttributeIDs::getNormalName() { return "Normal"; }
	const std::string VertexAttributeIDs::getTangentName() { return "Tangent"; }
	const std::string VertexAttributeIDs::getBitangentName() { return "Bitangent"; }


	const std::string VertexAttributeIDs::getUVName(int uvChannel)
	{
		std::ostringstream stream;
		stream << "UV" << uvChannel;
		return stream.str();
	}


	const std::string VertexAttributeIDs::GetColorName(int colorChannel)
	{
		std::ostringstream stream;
		stream << "Color" << colorChannel;
		return stream.str();
	}
	
	//////////////////////////////////////////////////////////////////////////

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
			mGPUMesh->addVertexAttribute(mesh_attribute->mAttributeID, mesh_attribute->getType(), mesh_attribute->getNumComponents(), GL_STATIC_DRAW);

		return update(errorState);
	}


	bool MeshInstance::init(utility::ErrorState& errorState)
	{
		return initGPUData(errorState);
	}


	void MeshInstance::copyMeshProperties(RTTIMeshProperties& meshProperties)
	{
		mProperties.mAttributes.reserve(meshProperties.mAttributes.size());
		for (auto& mesh_attribute : meshProperties.mAttributes)
		{
			std::unique_ptr<BaseVertexAttribute> owned_mesh_attribute(mesh_attribute->get_type().create<BaseVertexAttribute>());
			rtti::copyObject(*mesh_attribute, *owned_mesh_attribute);
			mProperties.mAttributes.emplace_back(std::move(owned_mesh_attribute));
		}
		mProperties.mNumVertices = meshProperties.mNumVertices;

		mProperties.mShapes.resize(meshProperties.mShapes.size());
		for (int index = 0; index < meshProperties.mShapes.size(); ++index)
		{
			MeshShape& source_shape = meshProperties.mShapes[index];
			MeshShape& dest_shape = mProperties.mShapes[index];

			assert(source_shape.getNumIndices() != 0);

			dest_shape.setDrawMode(source_shape.getDrawMode());
			dest_shape.setIndices(source_shape.getIndices().data(), source_shape.getIndices().size());
		}
	}


	void MeshInstance::reserveVertices(size_t numVertices)
	{
		for (auto& mesh_attribute : mProperties.mAttributes)
			mesh_attribute->reserve(numVertices);
	}


	MeshShape& MeshInstance::createShape()
	{
		mProperties.mShapes.push_back(MeshShape());
		return mProperties.mShapes.back();
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
			opengl::VertexAttributeBuffer& vertex_attr_buffer = mGPUMesh->getVertexAttributeBuffer(mesh_attribute->mAttributeID);
			vertex_attr_buffer.setData(mesh_attribute->getRawData(), mesh_attribute->getCount(), mesh_attribute->getCapacity());
		}


		for (int shapeIndex = 0; shapeIndex != mProperties.mShapes.size(); ++shapeIndex)
			mGPUMesh->getOrCreateIndexBuffer(shapeIndex).setData(mProperties.mShapes[shapeIndex].getIndices());

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool Mesh::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mProperties.mShapes.empty(), "Mesh %s has no sub meshes", mID.c_str()))
			return false;

		for (int index = 0; index < mProperties.mShapes.size(); ++index)
		{
			MeshShape& shape = mProperties.mShapes[index];
			if (shape.getNumIndices() == 0)
				generateIndices(shape, mProperties.mNumVertices);
		}

		mMeshInstance.copyMeshProperties(mProperties);
		return mMeshInstance.init(errorState);
	}
}