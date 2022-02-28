/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "particlevolumecomponent.h"

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
#include <storageuniforminstance.h>

RTTI_BEGIN_CLASS(nap::ParticleVolumeComponent)
	RTTI_PROPERTY("NumParticles",				&nap::ParticleVolumeComponent::mNumParticles,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",						&nap::ParticleVolumeComponent::mSize,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationSpeed",				&nap::ParticleVolumeComponent::mRotationSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",						&nap::ParticleVolumeComponent::mDisplacement,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TimeScale",					&nap::ParticleVolumeComponent::mTimeScale,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationVariation",			&nap::ParticleVolumeComponent::mRotationVariation,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParticleVolumeComponentInstance)
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
		constexpr const char* deltaTime = "deltaTime";
		constexpr const char* elapsedTime = "elapsedTime";

		constexpr const char* displacement = "displacement";
		constexpr const char* rotationSpeed = "rotationSpeed";
		constexpr const char* rotationVariation = "rotationVariation";
		constexpr const char* particleSize = "particleSize";

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
	// ParticleMesh
	//////////////////////////////////////////////////////////////////////////

	ParticleMesh::ParticleMesh(Core& core) : mRenderService(core.getService<RenderService>())
	{ }

	bool ParticleMesh::init(utility::ErrorState& errorState)
	{
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		int num_vertices = mNumParticles * 4;
		mMeshInstance->setNumVertices(num_vertices);
		mMeshInstance->setUsage(EMemoryUsage::Static);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(ECullMode::None);

		// Create the necessary attributes
		Vec4VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::position);
		Vec4VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getUVName(0));
		UIntVertexAttribute& id_attribute = mMeshInstance->getOrCreateAttribute<uint>(vertexid::id);
		MeshShape& shape = mMeshInstance->createShape();

		// Reserve CPU memory for all the particle geometry necessary to create
		// We want to draw the mesh as a set of triangles, 2 triangles per particle
		shape.reserveIndices(mNumParticles);

		// Build the mesh based on the amount of particles
		std::vector<glm::vec4> zero_buffer(num_vertices);
		position_attribute.setData(zero_buffer.data(), num_vertices);

		uint cur_num_vertices = 0;
		for (int i = 0; i < mNumParticles; i++)
		{
			uv_attribute.addData(plane_uvs, 4);

			uint id = (cur_num_vertices - cur_num_vertices % 4)/4;
			std::array<uint, 4> id_array = { id, id, id, id };
			id_attribute.addData(id_array.data(), 4);

			// Indices for 2 triangles, 1 plane
			uint indices[] =
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
	}


	//////////////////////////////////////////////////////////////////////////
	// ParticleVolumeComponentInstance
	//////////////////////////////////////////////////////////////////////////

	ParticleVolumeComponentInstance::ParticleVolumeComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mParticleMesh(std::make_unique<ParticleMesh>(*entity.getCore())),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }


	bool ParticleVolumeComponentInstance::init(utility::ErrorState& errorState)
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
		ParticleVolumeComponent* resource = getComponent<ParticleVolumeComponent>();

		mParticleSize = resource->mSize;
		mTimeScale = resource->mTimeScale;
		mDisplacement = resource->mDisplacement;
		mRotationVariation = resource->mRotationVariation;
		mRotationSpeed = resource->mRotationSpeed;

		for (auto& comp : mComputeInstances)
			comp->setInvocations(resource->mNumParticles);

		// Initialize particle mesh
		mParticleMesh->mNumParticles = resource->mNumParticles;
		if (!errorState.check(mParticleMesh->init(errorState), "Unable to create particle mesh"))
			return false;

		// Bind the particle mesh to the material and create a VAO
		RenderableMesh renderableMesh = createRenderableMesh(*mParticleMesh, errorState);
		if (!renderableMesh.isValid())
			return false;

		// Set the particle mesh to be used when drawing
		setMesh(renderableMesh);

		return true;
	}


	void ParticleVolumeComponentInstance::update(double deltaTime)
	{
		// Update time
		mDeltaTime = deltaTime * static_cast<double>(mTimeScale);
		mElapsedTime += mDeltaTime;		
	}


	void ParticleVolumeComponentInstance::compute()
	{
		if (!mFirstUpdate)
		{
			mComputeInstanceIndex = (mComputeInstanceIndex + 1) % mComputeInstances.size();
			mCurrentComputeInstance = mComputeInstances[mComputeInstanceIndex];
		}
		mFirstUpdate = false;

		// Update compute shader uniforms
		UniformStructInstance* ubo_struct = mCurrentComputeInstance->getComputeMaterialInstance().getOrCreateUniform(uniform::uboStruct);
		if (ubo_struct != nullptr)
		{
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::elapsedTime)->setValue(static_cast<float>(mElapsedTime));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::deltaTime)->setValue(static_cast<float>(mDeltaTime));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::displacement)->setValue(mDisplacement);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::rotationSpeed)->setValue(mRotationSpeed);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::rotationVariation)->setValue(mRotationVariation);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::particleSize)->setValue(mParticleSize);
		}

		mRenderService->computeObjects({ mCurrentComputeInstance });
	}


	void ParticleVolumeComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);
		
		// Find storage buffer uniform in the material instance resource, else the material resource
		StorageUniformStructInstance* vertex_struct = mCurrentComputeInstance->getComputeMaterialInstance().findStorageUniform(uniform::vertexBufferStruct);
		if (vertex_struct == nullptr)
			vertex_struct = mCurrentComputeInstance->getComputeMaterialInstance().getBaseMaterial()->findStorageUniform(uniform::vertexBufferStruct);

		// Get storage buffer handle
		StorageUniformVec4BufferInstance* vertex_buffer_uniform = vertex_struct->getOrCreateStorageUniformBuffer<StorageUniformVec4BufferInstance>(uniform::vertices);
		const VkBuffer storage_buffer = vertex_buffer_uniform->getTypedBuffer().getBuffer();

		// Override position vertex attribute buffer with storage buffer
		std::vector<VkBuffer> vertex_buffers = mRenderableMesh.getVertexBuffers();
		int position_attr_binding_idx = mRenderableMesh.getVertexBufferBindingIndex(vertexid::position);
		vertex_buffers[position_attr_binding_idx] = storage_buffer;

		// Get offsets
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

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
		const MeshInstance& mesh_instance = getMeshInstance();
		const GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();
		const IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(0);

		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

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
