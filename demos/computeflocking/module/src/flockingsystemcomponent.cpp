/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define GLM_FORCE_SWIZZLE

// Local Includes
#include "flockingsystemcomponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <glm/gtx/transform.hpp>
#include <mathutils.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <nap/logger.h>
#include <descriptorsetcache.h>

RTTI_BEGIN_CLASS(nap::FlockingSystemComponent)
	RTTI_PROPERTY("NumBoids",					&nap::FlockingSystemComponent::mNumBoids,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Target",						&nap::FlockingSystemComponent::mTargetParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BoidSize",					&nap::FlockingSystemComponent::mBoidSizeParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ViewRadius",					&nap::FlockingSystemComponent::mViewRadiusParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AvoidRadius",				&nap::FlockingSystemComponent::mAvoidRadiusParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinSpeed",					&nap::FlockingSystemComponent::mMinSpeedParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxSpeed",					&nap::FlockingSystemComponent::mMaxSpeedParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxSteerForce",				&nap::FlockingSystemComponent::mMaxSteerForceParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TargetWeight",				&nap::FlockingSystemComponent::mTargetWeightParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AlignmentWeight",			&nap::FlockingSystemComponent::mAlignmentWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CohesionWeight",				&nap::FlockingSystemComponent::mCohesionWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SeparationWeight",			&nap::FlockingSystemComponent::mSeparationWeightParam,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlockingSystemComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		constexpr const char* uboStruct = "UBO";

		constexpr const char* target = "target";
		constexpr const char* deltaTime = "deltaTime";
		constexpr const char* elapsedTime = "elapsedTime";
		constexpr const char* boidSize = "boidSize";
		constexpr const char* viewRadius = "viewRadius";
		constexpr const char* avoidRadius = "avoidRadius";
		constexpr const char* minSpeed = "minSpeed";
		constexpr const char* maxSpeed = "maxSpeed";
		constexpr const char* maxSteerForce = "maxSteerForce";
		constexpr const char* targetWeight = "targetWeight";
		constexpr const char* alignmentWeight = "alignmentWeight";
		constexpr const char* cohesionWeight = "cohesionWeight";
		constexpr const char* separationWeight = "separationWeight";
		constexpr const char* numBoids = "numBoids";

		constexpr const char* vertexBufferStruct = "VertexBuffer";
		constexpr const char* vertices = "vertices";
	}

	namespace vertexid
	{
		constexpr const char* id = "Id";
	}


	//////////////////////////////////////////////////////////////////////////
	// Static functions and data
	//////////////////////////////////////////////////////////////////////////

	// Default normalized plane uv's
	static glm::vec4 plane_uvs[] =
	{
		{ 0.0f,	0.0f, 0.0f, 0.0f },
		{ 1.0f,	0.0f, 0.0f, 0.0f },
		{ 0.0f,	1.0f, 0.0f, 0.0f },
		{ 1.0f,	1.0f, 0.0f, 0.0f },
	};

	//////////////////////////////////////////////////////////////////////////
	// BoidMesh
	//////////////////////////////////////////////////////////////////////////

	BoidMesh::BoidMesh(Core& core) : mRenderService(core.getService<RenderService>())
	{ }

	bool BoidMesh::init(utility::ErrorState& errorState)
	{
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		int num_vertices = mNumBoids * 4;
		mMeshInstance->setNumVertices(num_vertices);
		mMeshInstance->setUsage(EMeshDataUsage::Static);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(ECullMode::None);

		// Create the necessary attributes
		Vec4VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::position);
		Vec4VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getUVName(0));
		IntVertexAttribute& id_attribute = mMeshInstance->getOrCreateAttribute<int>(vertexid::id);
		MeshShape& shape = mMeshInstance->createShape();

		// Reserve CPU memory for all the particle geometry necessary to create
		// We want to draw the mesh as a set of triangles, 2 triangles per particle
		shape.reserveIndices(mNumBoids);

		// Build the mesh based on the amount of particles
		std::vector<glm::vec4> zero_buffer(num_vertices);
		position_attribute.setData(zero_buffer.data(), num_vertices);

		uint cur_num_vertices = 0;
		for (int i = 0; i < mNumBoids; i++)
		{
			uv_attribute.addData(plane_uvs, 4);

			int id = (cur_num_vertices - cur_num_vertices % 4) / 4;
			std::array<int, 4> id_array = { id, id, id, id };
			id_attribute.addData(id_array.data(), 4);

			// Indices for 2 triangles, 1 plane
			unsigned int indices[] =
			{
				cur_num_vertices + 0,
				cur_num_vertices + 1,
				cur_num_vertices + 3,
				cur_num_vertices + 0,
				cur_num_vertices + 3,
				cur_num_vertices + 2
			};
			shape.addIndices(indices, 6);
			cur_num_vertices += 4;
		}

		// Initialize our instance
		return mMeshInstance->init(errorState);
	};


	//////////////////////////////////////////////////////////////////////////
	// FlockingSystemComponentInstance
	//////////////////////////////////////////////////////////////////////////

	FlockingSystemComponentInstance::FlockingSystemComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mBoidMesh(std::make_unique<BoidMesh>(*entity.getCore())),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }


	bool FlockingSystemComponentInstance::init(utility::ErrorState& errorState)
	{
		// Ensure a compute component is available
		if (!errorState.check(getEntityInstance()->findComponent<ComputeComponentInstance>() != nullptr, "%s: missing ComputeComponent", mID.c_str()))
			return false;

		// Collect compute instances
		getEntityInstance()->getComponentsOfType<ComputeComponentInstance>(mComputeInstances);
		mCurrentComputeInstance = mComputeInstances[mComputeInstanceIndex];

		// Initialize base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Get resource
		FlockingSystemComponent* resource = getComponent<FlockingSystemComponent>();

		// Set params from resource
		mTarget = glm::vec4(resource->mTargetParam->mValue, 0.0f);
		mBoidSize = resource->mBoidSizeParam->mValue;
		mViewRadius = resource->mViewRadiusParam->mValue;
		mAvoidRadius = resource->mAvoidRadiusParam->mValue;
		mMinSpeed = resource->mMinSpeedParam->mValue;
		mMaxSpeed = resource->mMaxSpeedParam->mValue;
		mMaxSteerForce = resource->mMaxSteerForceParam->mValue;
		mTargetWeight = resource->mTargetWeightParam->mValue;
		mAlignmentWeight = resource->mAlignmentWeightParam->mValue;
		mCohesionWeight = resource->mCohesionWeightParam->mValue;
		mSeparationWeight = resource->mSeparationWeightParam->mValue;
		mNumBoids = resource->mNumBoids;

		// Initialize particle mesh
		mBoidMesh->mNumBoids = resource->mNumBoids;
		if (!errorState.check(mBoidMesh->init(errorState), "Unable to create particle mesh"))
			return false;

		// Bind the particle mesh to the material and create a VAO
		RenderableMesh renderableMesh = createRenderableMesh(*mBoidMesh, errorState);
		if (!renderableMesh.isValid())
			return false;

		// Set the particle mesh to be used when drawing
		setMesh(renderableMesh);

		return true;
	}


	void FlockingSystemComponentInstance::update(double deltaTime)
	{
		mElapsedTime += deltaTime;

		if (!mFirstUpdate)
		{
			mComputeInstanceIndex = (mComputeInstanceIndex + 1) % mComputeInstances.size();
			mCurrentComputeInstance = mComputeInstances[mComputeInstanceIndex];
		}

		// Update uniforms
		UniformStructInstance* ubo_struct = mCurrentComputeInstance->getComputeMaterialInstance().getOrCreateUniform(uniform::uboStruct);
		if (ubo_struct != nullptr)
		{
			FlockingSystemComponent* resource = getComponent<FlockingSystemComponent>();

			ubo_struct->getOrCreateUniform<UniformVec4Instance>(uniform::target)->setValue(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::elapsedTime)->setValue(static_cast<float>(mElapsedTime));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::deltaTime)->setValue(static_cast<float>(deltaTime));

			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::boidSize)->setValue(resource->mBoidSizeParam->mValue);

			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::viewRadius)->setValue(mViewRadius);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::avoidRadius)->setValue(mAvoidRadius);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::minSpeed)->setValue(mMinSpeed);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::maxSpeed)->setValue(mMaxSpeed);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::maxSteerForce)->setValue(mMaxSteerForce);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::targetWeight)->setValue(mTargetWeight);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::alignmentWeight)->setValue(mAlignmentWeight);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::cohesionWeight)->setValue(mCohesionWeight);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::separationWeight)->setValue(mSeparationWeight);
			ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::numBoids)->setValue(mNumBoids);
		}
		mFirstUpdate = false;
	}


	bool FlockingSystemComponentInstance::compute(utility::ErrorState& errorState)
	{
		if (!mCurrentComputeInstance->compute(mBoidMesh->mNumBoids, errorState))
			return false;

		return true;
	}


	void FlockingSystemComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		VkDescriptorSet descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();

		// Find storage buffer uniform in the material instance resource, else the material resource
		StorageUniformStructInstance* vertex_struct = mCurrentComputeInstance->getComputeMaterialInstance().findStorageUniform(uniform::vertexBufferStruct);
		if (vertex_struct == nullptr)
			vertex_struct = mCurrentComputeInstance->getComputeMaterialInstance().getBaseMaterial()->findStorageUniform(uniform::vertexBufferStruct);

		StorageUniformVec4BufferInstance* vertex_buffer_uniform = vertex_struct->getOrCreateStorageUniform<StorageUniformVec4BufferInstance>(uniform::vertices);
		const VkBuffer storage_buffer = vertex_buffer_uniform->getTypedValueBuffer().getBuffer();

		std::vector<VkBuffer> vertex_buffers_override = { storage_buffer, vertex_buffers[1], vertex_buffers[2] };
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers_override.size(), vertex_buffers_override.data(), offsets.data());

		// TODO: move to push/pop cliprect on RenderTarget once it has been ported
		bool has_clip_rect = mClipRect.hasWidth() && mClipRect.hasHeight();
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);

		// Restore clipping
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getBufferSize().x;
			rect.extent.height = renderTarget.getBufferSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}
	}
}
