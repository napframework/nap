// Local Includes
#include "mesh.h"
#include "meshutils.h"
#include "renderservice.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

// External Includes
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <nap/assert.h>

RTTI_BEGIN_ENUM(nap::EDrawMode)
	RTTI_ENUM_VALUE(nap::EDrawMode::Unknown,		"Unknown"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Points,			"Points"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Lines,			"Lines"),
	RTTI_ENUM_VALUE(nap::EDrawMode::LineStrip,		"LineStrip"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Triangles,		"Triangles"),
	RTTI_ENUM_VALUE(nap::EDrawMode::TriangleStrip,	"TriangleStrip"),
	RTTI_ENUM_VALUE(nap::EDrawMode::TriangleFan,	"TriangleFan")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::MeshShape)
	RTTI_PROPERTY("Indices",		&nap::MeshShape::mIndices,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RTTIMeshProperties)
	RTTI_PROPERTY("NumVertices",	&nap::RTTIMeshProperties::mNumVertices,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",			&nap::RTTIMeshProperties::mUsage,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DrawMode",		&nap::RTTIMeshProperties::mDrawMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::RTTIMeshProperties::mCullMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attributes",		&nap::RTTIMeshProperties::mAttributes,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shapes",			&nap::RTTIMeshProperties::mShapes,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS(nap::Mesh)
	RTTI_CONSTRUCTOR(nap::Core&)
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
	// MeshInstance
	//////////////////////////////////////////////////////////////////////////

	MeshInstance::MeshInstance(RenderService* renderService) :
		mRenderService(renderService)
	{ }


	MeshInstance::~MeshInstance()
	{ }


	// Returns associated mesh
	GPUMesh& MeshInstance::getGPUMesh() const
	{
		assert(mGPUMesh != nullptr);
		return *mGPUMesh;
	}


	// Creates GPU vertex attributes and updates mesh
	bool MeshInstance::initGPUData(utility::ErrorState& errorState)
	{
		mGPUMesh = std::make_unique<GPUMesh>(*mRenderService, mProperties.mUsage);
		for (auto& mesh_attribute : mProperties.mAttributes)
			mGPUMesh->addVertexAttribute(mesh_attribute->mAttributeID, mesh_attribute->getFormat());

		return update(errorState);
	}


	bool MeshInstance::init(utility::ErrorState& errorState)
	{
		mInitialized = initGPUData(errorState);
		return mInitialized;
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
		// Assert when trying to update a mesh that is static and already initialized
		NAP_ASSERT_MSG(!mInitialized || mProperties.mUsage == EMeshDataUsage::DynamicWrite, 
			"trying to update mesh without usage set to: 'DynamicWrite'");

		// Check for mismatches in sizes
		for (auto& mesh_attribute : mProperties.mAttributes)
		{
			if (!errorState.check(mesh_attribute->getCount() == mProperties.mNumVertices,
				"Vertex attribute %s has a different amount of elements (%d) than the mesh (%d)", mesh_attribute->mAttributeID.c_str(), mesh_attribute->getCount(), mProperties.mNumVertices))
			{
				return false;
			}
		}

		// Synchronize mesh attributes
		for (auto& mesh_attribute : mProperties.mAttributes)
		{
			VertexAttributeBuffer& vertex_attr_buffer = mGPUMesh->getVertexAttributeBuffer(mesh_attribute->mAttributeID);
			vertex_attr_buffer.setData(mRenderService->getPhysicalDevice(), mRenderService->getDevice(), mesh_attribute->getRawData(), mesh_attribute->getCount(), mesh_attribute->getCapacity());
		}

		// Synchronize mesh indices
		for (int shapeIndex = 0; shapeIndex != mProperties.mShapes.size(); ++shapeIndex)
		{
			mGPUMesh->getOrCreateIndexBuffer(shapeIndex).setData(mRenderService->getPhysicalDevice(), mRenderService->getDevice(), mProperties.mShapes[shapeIndex].getIndices());
		}

		return true;
	}


	bool MeshInstance::update(nap::BaseVertexAttribute& attribute, utility::ErrorState& errorState)
	{
		VertexAttributeBuffer& gpu_buffer = mGPUMesh->getVertexAttributeBuffer(attribute.mAttributeID);
		if (!errorState.check(attribute.getCount() == mProperties.mNumVertices,
			"Vertex attribute %s has a different amount of elements (%d) than the mesh (%d)", attribute.mAttributeID.c_str(), attribute.getCount(), mProperties.mNumVertices))
		{
			return false;
		}
		gpu_buffer.setData(mRenderService->getPhysicalDevice(), mRenderService->getDevice(), attribute.getRawData(), attribute.getCount(), attribute.getCapacity());
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Mesh
	//////////////////////////////////////////////////////////////////////////

	Mesh::Mesh(Core& core) : mRenderService(core.getService<nap::RenderService>())
	{}


	bool Mesh::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mProperties.mShapes.empty(), "Mesh %s has no sub meshes", mID.c_str()))
			return false;

		for (int index = 0; index < mProperties.mShapes.size(); ++index)
		{
			MeshShape& shape = mProperties.mShapes[index];
			if (shape.getNumIndices() == 0)
				utility::generateIndices(shape, mProperties.mNumVertices);
		}

		// Create instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<nap::MeshInstance>(mRenderService);
		
		// Copy properties from resource to instance and initialize
		mMeshInstance->copyMeshProperties(mProperties);
		return mMeshInstance->init(errorState);
	}


	nap::MeshInstance& Mesh::getMeshInstance()
	{
		assert(mMeshInstance != nullptr);
		return *mMeshInstance;
	}


	const nap::MeshInstance& nap::Mesh::getMeshInstance() const
	{
		assert(mMeshInstance != nullptr);
		return *mMeshInstance;
	}

}
