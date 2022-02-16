/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define GLM_FORCE_SWIZZLE

// Local Includes
#include "flockingsystemcomponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <mathutils.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <nap/logger.h>
#include <descriptorsetcache.h>

RTTI_BEGIN_CLASS(nap::FlockingSystemComponent)
	RTTI_PROPERTY("NumBoids",					&nap::FlockingSystemComponent::mNumBoids,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomColor",				&nap::FlockingSystemComponent::mRandomColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BoidSize",					&nap::FlockingSystemComponent::mBoidSizeParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FresnelScale",				&nap::FlockingSystemComponent::mFresnelScaleParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FresnelPower",				&nap::FlockingSystemComponent::mFresnelPowerParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ViewRadius",					&nap::FlockingSystemComponent::mViewRadiusParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AvoidRadius",				&nap::FlockingSystemComponent::mAvoidRadiusParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinSpeed",					&nap::FlockingSystemComponent::mMinSpeedParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxSpeed",					&nap::FlockingSystemComponent::mMaxSpeedParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxSteerForce",				&nap::FlockingSystemComponent::mMaxSteerForceParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TargetWeight",				&nap::FlockingSystemComponent::mTargetWeightParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AlignmentWeight",			&nap::FlockingSystemComponent::mAlignmentWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CohesionWeight",				&nap::FlockingSystemComponent::mCohesionWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SeparationWeight",			&nap::FlockingSystemComponent::mSeparationWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BoundsRadius",				&nap::FlockingSystemComponent::mBoundsRadiusParam,			nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("LightPosition",				&nap::FlockingSystemComponent::mLightPositionParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LightIntensity",				&nap::FlockingSystemComponent::mLightIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DiffuseColor",				&nap::FlockingSystemComponent::mDiffuseColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DiffuseColorEx",				&nap::FlockingSystemComponent::mDiffuseColorExParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LightColor",					&nap::FlockingSystemComponent::mLightColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HaloColor",					&nap::FlockingSystemComponent::mHaloColorParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpecularColor",				&nap::FlockingSystemComponent::mSpecularColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Shininess",					&nap::FlockingSystemComponent::mShininessParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AmbientIntensity",			&nap::FlockingSystemComponent::mAmbientIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DiffuseIntensity",			&nap::FlockingSystemComponent::mDiffuseIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpecularIntensity",			&nap::FlockingSystemComponent::mSpecularIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MateColorRate",				&nap::FlockingSystemComponent::mMateColorRateParam,			nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("PerspCameraComponent",		&nap::FlockingSystemComponent::mPerspCameraComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetTransformComponent",	&nap::FlockingSystemComponent::mTargetTransformComponent,	nap::rtti::EPropertyMetaData::Required)
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
		constexpr const char* vertUboStruct = "Vert_UBO";
		constexpr const char* randomColor = "randomColor";
		constexpr const char* boidSize = "boidSize";
		constexpr const char* cameraLocation = "cameraLocation";
		constexpr const char* lightPosition = "lightPosition";
		constexpr const char* lightIntensity = "lightIntensity";
		constexpr const char* diffuseColor = "diffuseColor";
		constexpr const char* diffuseColorEx = "diffuseColorEx";
		constexpr const char* lightColor = "lightColor";
		constexpr const char* haloColor = "haloColor";
		constexpr const char* specularIntensity = "specularIntensity";
		constexpr const char* specularColor = "specularColor";
		constexpr const char* mateColorRate = "mateColorRate";
		constexpr const char* shininess = "shininess";
		constexpr const char* ambientIntensity = "ambientIntensity";
		constexpr const char* diffuseIntensity = "diffuseIntensity";
		constexpr const char* fresnelScale = "fresnelScale";
		constexpr const char* fresnelPower = "fresnelPower";
	}

	namespace computeuniform
	{
		constexpr const char* uboStruct = "UBO";
		constexpr const char* target = "target";
		constexpr const char* deltaTime = "deltaTime";
		constexpr const char* elapsedTime = "elapsedTime";
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
		constexpr const char* matrixBufferStruct = "MatrixBuffer";
		constexpr const char* transforms = "transforms";
		constexpr const char* boundsRadius = "boundsRadius";
	}

	namespace vertexid
	{
		constexpr const char* id = "Id";
	}


	//////////////////////////////////////////////////////////////////////////
	// FlockingSystemComponentInstance
	//////////////////////////////////////////////////////////////////////////

	FlockingSystemComponentInstance::FlockingSystemComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }


	bool FlockingSystemComponentInstance::init(utility::ErrorState& errorState)
	{
		// Ensure a compute component is available
		if (!errorState.check(getEntityInstance()->findComponent<ComputeComponentInstance>() != nullptr, "%s: missing ComputeComponent", mID.c_str()))
			return false;

		// Cache resource
		mResource = getComponent<FlockingSystemComponent>();

		// Collect compute instances
		getEntityInstance()->getComponentsOfType<ComputeComponentInstance>(mComputeInstances);
		mCurrentComputeInstance = mComputeInstances[mComputeInstanceIndex];

		// Initialize base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Set non-parameter variables
		mNumBoids = mResource->mNumBoids;

		for (auto& comp : mComputeInstances)
			comp->setInvocations(mNumBoids);

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

		// Update compute shader uniforms
		UniformStructInstance* ubo_struct = mCurrentComputeInstance->getComputeMaterialInstance().getOrCreateUniform(uniform::uboStruct);
		if (ubo_struct != nullptr)
		{
			glm::vec4 target_position = mTargetTransformComponent->getGlobalTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(computeuniform::target)->setValue(target_position.xyz);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::elapsedTime)->setValue(static_cast<float>(mElapsedTime));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::deltaTime)->setValue(static_cast<float>(deltaTime));
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::viewRadius)->setValue(mResource->mViewRadiusParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::avoidRadius)->setValue(mResource->mAvoidRadiusParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::minSpeed)->setValue(mResource->mMinSpeedParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::maxSpeed)->setValue(mResource->mMaxSpeedParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::maxSteerForce)->setValue(mResource->mMaxSteerForceParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::targetWeight)->setValue(mResource->mTargetWeightParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::alignmentWeight)->setValue(mResource->mAlignmentWeightParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::cohesionWeight)->setValue(mResource->mCohesionWeightParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::separationWeight)->setValue(mResource->mSeparationWeightParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::boundsRadius)->setValue(mResource->mBoundsRadiusParam->mValue);
			ubo_struct->getOrCreateUniform<UniformUIntInstance>(computeuniform::numBoids)->setValue(mResource->mNumBoids);
		}

		auto& camera_transform = mPerspCameraComponent->getEntityInstance()->getComponent<TransformComponentInstance>();

		// Update vertex shader uniforms
		ubo_struct = getMaterialInstance().getOrCreateUniform(uniform::vertUboStruct);
		if (ubo_struct != nullptr)
		{
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::boidSize)->setValue(mResource->mBoidSizeParam->mValue);
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::cameraLocation)->setValue(camera_transform.getTranslate());
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::fresnelScale)->setValue(mResource->mFresnelScaleParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::fresnelPower)->setValue(mResource->mFresnelPowerParam->mValue);
		}

		// Update vertex shader storage uniforms
		auto* ssbo_struct = getMaterialInstance().findStorageUniform("SSBO");
		auto* compute_struct = mCurrentComputeInstance->getComputeMaterialInstance().findStorageUniform("BoidBuffer_Out");
		if (ssbo_struct != nullptr && compute_struct != nullptr)
		{
			auto& storage_buffer = compute_struct->findStorageUniformBuffer<StorageUniformStructBufferInstance>("boids")->getBuffer();
			ssbo_struct->findStorageUniformBuffer<StorageUniformStructBufferInstance>("boids")->setBuffer(storage_buffer);
		}

		// Update fragment shader uniforms
		ubo_struct = getMaterialInstance().getOrCreateUniform(uniform::uboStruct);
		if (ubo_struct != nullptr)
		{
			ubo_struct->getOrCreateUniform<UniformUIntInstance>(uniform::randomColor)->setValue(mResource->mRandomColorParam->mValue);
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::cameraLocation)->setValue(camera_transform.getTranslate());
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition)->setValue(mResource->mLightPositionParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightIntensity)->setValue(mResource->mLightIntensityParam->mValue);
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::diffuseColor)->setValue(mResource->mDiffuseColorParam->mValue.toVec3());
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::diffuseColorEx)->setValue(mResource->mDiffuseColorExParam->mValue.toVec3());
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightColor)->setValue(mResource->mLightColorParam->mValue.toVec3());
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::haloColor)->setValue(mResource->mHaloColorParam->mValue.toVec3());
			ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::specularColor)->setValue(mResource->mSpecularColorParam->mValue.toVec3());
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::shininess)->setValue(mResource->mShininessParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::ambientIntensity)->setValue(mResource->mAmbientIntensityParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::diffuseIntensity)->setValue(mResource->mDiffuseIntensityParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::specularIntensity)->setValue(mResource->mSpecularIntensityParam->mValue);
			ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::mateColorRate)->setValue(mResource->mMateColorRateParam->mValue);
		}

		mFirstUpdate = false;
	}


	void FlockingSystemComponentInstance::compute()
	{
		mRenderService->computeObjects({ mCurrentComputeInstance });
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
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();
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
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		const IndexBuffer& index_buffer = mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), mNumBoids, 0, 0, 0);

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


	FlockingSystemComponent& FlockingSystemComponentInstance::getResource()
	{
		return *mResource;
	}
}
