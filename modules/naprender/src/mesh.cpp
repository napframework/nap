/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
#include <nap/logger.h>

RTTI_BEGIN_ENUM(nap::EDrawMode)
	RTTI_ENUM_VALUE(nap::EDrawMode::Unknown,		"Unknown"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Points,			"Points"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Lines,			"Lines"),
	RTTI_ENUM_VALUE(nap::EDrawMode::LineStrip,		"LineStrip"),
	RTTI_ENUM_VALUE(nap::EDrawMode::Triangles,		"Triangles"),
	RTTI_ENUM_VALUE(nap::EDrawMode::TriangleStrip,	"TriangleStrip"),
	RTTI_ENUM_VALUE(nap::EDrawMode::TriangleFan,	"TriangleFan")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ECullMode)
	RTTI_ENUM_VALUE(nap::ECullMode::Back,			"Back"),
	RTTI_ENUM_VALUE(nap::ECullMode::Front,			"Front"),
	RTTI_ENUM_VALUE(nap::ECullMode::FrontAndBack,	"FrontAndBack"),
	RTTI_ENUM_VALUE(nap::ECullMode::None,			"None")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EPolygonMode)
	RTTI_ENUM_VALUE(nap::EPolygonMode::Fill,		"Fill"),
	RTTI_ENUM_VALUE(nap::EPolygonMode::Line,		"Line"),
	RTTI_ENUM_VALUE(nap::EPolygonMode::Point,		"Point")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::MeshShape)
	RTTI_PROPERTY("Indices",		&nap::MeshShape::mIndices,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RTTIMeshProperties)
	RTTI_PROPERTY("NumVertices",	&nap::RTTIMeshProperties::mNumVertices,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",			&nap::RTTIMeshProperties::mUsage,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DrawMode",		&nap::RTTIMeshProperties::mDrawMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::RTTIMeshProperties::mCullMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::RTTIMeshProperties::mPolygonMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attributes",		&nap::RTTIMeshProperties::mAttributes,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shapes",			&nap::RTTIMeshProperties::mShapes,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS(nap::Mesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Properties",		&nap::Mesh::mProperties,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMesh)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// MeshInstance
	//////////////////////////////////////////////////////////////////////////

	MeshInstance::MeshInstance(RenderService& renderService) :
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
		mGPUMesh = std::make_unique<GPUMesh>(mRenderService, mProperties.mUsage);
		for (auto& mesh_attribute : mProperties.mAttributes)
		{
			VertexBuffer* buffer = nullptr;

			//if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<uint8>))
			//	buffer = &mGPUMesh->addVertexBuffer<uint8>(mesh_attribute->mAttributeID);

			if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<int>))
				buffer = &mGPUMesh->addVertexBuffer<int>(mesh_attribute->mAttributeID);

			else if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<float>))
				buffer = &mGPUMesh->addVertexBuffer<float>(mesh_attribute->mAttributeID);

			else if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<glm::vec2>))
				buffer = &mGPUMesh->addVertexBuffer<glm::vec2>(mesh_attribute->mAttributeID);

			else if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<glm::vec3>))
				buffer = &mGPUMesh->addVertexBuffer<glm::vec3>(mesh_attribute->mAttributeID);

			else if (mesh_attribute->get_type() == RTTI_OF(VertexAttribute<glm::vec4>))
				buffer = &mGPUMesh->addVertexBuffer<glm::vec4>(mesh_attribute->mAttributeID);

			else
			{
				errorState.fail("Unsupported vertex buffer type");
				return false;
			}

			// Initialize the added vertex attribute buffer
			buffer->mCount = mesh_attribute->getCount();
			if (!buffer->init(errorState))
				return false;
		}

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
		mProperties.mShapes.emplace_back(MeshShape());
		return mProperties.mShapes.back();
	}


	bool MeshInstance::update(utility::ErrorState& errorState)
	{
		// Assert when trying to update a mesh that is static and already initialized
		NAP_ASSERT_MSG(!mInitialized || mProperties.mUsage == EMeshDataUsage::DynamicWrite, 
			"trying to update previously allocated mesh without usage set to: 'DynamicWrite'");

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
			VertexBuffer& vertex_attr_buffer = mGPUMesh->getVertexBuffer(mesh_attribute->mAttributeID);
			if (!vertex_attr_buffer.setData(mesh_attribute->getRawData(), mesh_attribute->getCount(), mesh_attribute->getCapacity(), errorState))
				return false;
		}

		// Synchronize mesh indices
		for (int shapeIndex = 0; shapeIndex != mProperties.mShapes.size(); ++shapeIndex)
		{
			IndexBuffer& index_buffer = mGPUMesh->getOrCreateIndexBuffer(shapeIndex);
			if (!index_buffer.isInitialized())
			{
				index_buffer.mCount = mProperties.mShapes[shapeIndex].getNumIndices();
				if (!index_buffer.init(errorState))
					return false;
			}
			if (!index_buffer.setData(mProperties.mShapes[shapeIndex].getIndices(), errorState))
				return false;
		}
		return true;
	}


	bool MeshInstance::update(nap::BaseVertexAttribute& attribute, utility::ErrorState& errorState)
	{
		VertexBuffer& vertex_attr_buffer = mGPUMesh->getVertexBuffer(attribute.mAttributeID);
		if (!errorState.check(attribute.getCount() == mProperties.mNumVertices,
			"Vertex attribute %s has a different amount of elements (%d) than the mesh (%d)", attribute.mAttributeID.c_str(), attribute.getCount(), mProperties.mNumVertices))
		{
			return false;
		}
		return vertex_attr_buffer.setData(attribute.getRawData(), attribute.getCount(), attribute.getCapacity(), errorState);
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
		mMeshInstance = std::make_unique<nap::MeshInstance>(*mRenderService);
		
		// Copy properties from resource to instance and initialize
		mMeshInstance->copyMeshProperties(mProperties);
		return mMeshInstance->init(errorState);
	}


	void MeshShape::reserveIndices(size_t numIndices)
	{
		mIndices.reserve(numIndices);
	}


	void MeshShape::setIndices(uint32* indices, int numIndices)
	{
		mIndices.resize(numIndices);
		std::memcpy(mIndices.data(), indices, numIndices * sizeof(uint32));
	}


	void MeshShape::addIndices(uint32* indices, int numIndices)
	{
		size_t cur_num_indices = mIndices.size();
		mIndices.resize(cur_num_indices + (size_t)numIndices);
		std::memcpy(&mIndices[cur_num_indices], indices, (size_t)numIndices * sizeof(uint32));
	}


	void nap::MeshInstance::setPolygonMode(EPolygonMode mode)
	{
		/// Warn and return if the polygon mode is not supported.
		if(!mRenderService.getPolygonModeSupported(mode))
		{
			nap::Logger::warn("Selected polygon mode %s is not supported",
				RTTI_OF(EPolygonMode).get_enumeration().value_to_name(mode).to_string().c_str());
			return;
		}
		mProperties.mPolygonMode = mode;
	}
}
