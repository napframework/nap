/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "flockingsystemcomponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <mathutils.h>
#include <nap/core.h>
#include <nap/logger.h>
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
	RTTI_PROPERTY("TargetWeight",				&nap::FlockingSystemComponent::mTargetWeightParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AlignmentWeight",			&nap::FlockingSystemComponent::mAlignmentWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CohesionWeight",				&nap::FlockingSystemComponent::mCohesionWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SeparationWeight",			&nap::FlockingSystemComponent::mSeparationWeightParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BoundsRadius",				&nap::FlockingSystemComponent::mBoundsRadiusParam,			nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("LightPosition",				&nap::FlockingSystemComponent::mLightPositionParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LightIntensity",				&nap::FlockingSystemComponent::mLightIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DiffuseColor",				&nap::FlockingSystemComponent::mDiffuseColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LightColor",					&nap::FlockingSystemComponent::mLightColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HaloColor",					&nap::FlockingSystemComponent::mHaloColorParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpecularColor",				&nap::FlockingSystemComponent::mSpecularColorParam,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Shininess",					&nap::FlockingSystemComponent::mShininessParam,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DiffuseIntensity",			&nap::FlockingSystemComponent::mDiffuseIntensityParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpecularIntensity",			&nap::FlockingSystemComponent::mSpecularIntensityParam,		nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("TargetTransforms",			&nap::FlockingSystemComponent::mTargetTransforms,			nap::rtti::EPropertyMetaData::Default)
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
		constexpr const char* VERTUBO = "VERTUBO";
		constexpr const char* FRAGUBO = "FRAGUBO";
		constexpr const char* randomColor = "randomColor";
		constexpr const char* boidSize = "boidSize";
		constexpr const char* lightPosition = "lightPosition";
		constexpr const char* lightIntensity = "lightIntensity";
		constexpr const char* diffuseColor = "diffuseColor";
		constexpr const char* lightColor = "lightColor";
		constexpr const char* haloColor = "haloColor";
		constexpr const char* specularIntensity = "specularIntensity";
		constexpr const char* specularColor = "specularColor";
		constexpr const char* shininess = "shininess";
		constexpr const char* diffuseIntensity = "diffuseIntensity";
		constexpr const char* fresnelScale = "fresnelScale";
		constexpr const char* fresnelPower = "fresnelPower";
	}

	namespace computeuniform
	{
		constexpr const char* uboStruct = "UBO";
		constexpr const char* targets = "targets";
		constexpr const char* targetCount = "targetCount";
		constexpr const char* deltaTime = "deltaTime";
		constexpr const char* elapsedTime = "elapsedTime";
		constexpr const char* viewRadius = "viewRadius";
		constexpr const char* avoidRadius = "avoidRadius";
		constexpr const char* minSpeed = "minSpeed";
		constexpr const char* maxSpeed = "maxSpeed";
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

		// Fetch compute instance under this entity
		mComputeInstance = getEntityInstance()->findComponent<ComputeComponentInstance>();
		if (!errorState.check(mComputeInstance != nullptr, "Missing compute component"))
			return false;

		// Initialize base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Clamp the boid count if we are compiling on Raspberry Pi
#ifdef COMPUTEFLOCKING_RPI
		nap::Logger::info("Maximum boid count is limited to 1000 on Raspberry Pi to reduce perfomance issues");
		mNumBoids = math::clamp(mResource->mNumBoids, 0U, 1000U);
#else
		mNumBoids = mResource->mNumBoids;
#endif

		mBindingIn = mComputeInstance->getMaterialInstance().getOrCreateBuffer<BufferBindingStructInstance>("BoidBuffer_In");
		mBindingOut = mComputeInstance->getMaterialInstance().getOrCreateBuffer<BufferBindingStructInstance>("BoidBuffer_Out");
		mBoidBufferInput = &mBindingIn->getBuffer();
		mBoidBufferOutput = &mBindingOut->getBuffer();

		mComputeInstance->setInvocations(mNumBoids);
		auto* comp_ubo_struct = mComputeInstance->getMaterialInstance().getOrCreateUniform(computeuniform::uboStruct);
		if (comp_ubo_struct == nullptr)
			return false;

		mTargetsUniform = comp_ubo_struct->getOrCreateUniform<UniformVec3ArrayInstance>(computeuniform::targets);
		if (mTargetsUniform == nullptr)
			return false;

		mTargetCountUniform = comp_ubo_struct->getOrCreateUniform<UniformUIntInstance>(computeuniform::targetCount);
		if (mTargetCountUniform == nullptr)
			return false;

		mRenderStorageBinding = rtti_cast<BufferBindingStructInstance>(getMaterialInstance().findBinding("VERTSSBO"));
		if (mRenderStorageBinding == nullptr)
			return false;

		// Compute shader uniforms
		mElapsedTimeParam = std::make_unique<ParameterFloat>();
		mDeltaTimeParam = std::make_unique<ParameterFloat>();

		auto uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::elapsedTime);
		uni->setValue(mElapsedTimeParam->mValue);
		mElapsedTimeChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mElapsedTimeParam->valueChanged.connect(mElapsedTimeChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::deltaTime);
		uni->setValue(mDeltaTimeParam->mValue);
		mDeltaTimeChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mDeltaTimeParam->valueChanged.connect(mDeltaTimeChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::viewRadius);
		uni->setValue(mResource->mViewRadiusParam->mValue);
		mViewRadiusChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mViewRadiusParam->valueChanged.connect(mViewRadiusChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::avoidRadius);
		uni->setValue(mResource->mAvoidRadiusParam->mValue);
		mAvoidRadiusChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mAvoidRadiusParam->valueChanged.connect(mAvoidRadiusChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::minSpeed);
		uni->setValue(mResource->mMinSpeedParam->mValue);
		mMinSpeedChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mMinSpeedParam->valueChanged.connect(mMinSpeedChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::maxSpeed);
		uni->setValue(mResource->mMaxSpeedParam->mValue);
		mMaxSpeedChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mMaxSpeedParam->valueChanged.connect(mMaxSpeedChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::targetWeight);
		uni->setValue(mResource->mTargetWeightParam->mValue);
		mTargetWeightChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mTargetWeightParam->valueChanged.connect(mTargetWeightChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::alignmentWeight);
		uni->setValue(mResource->mAlignmentWeightParam->mValue);
		mAlignmentWeightChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mAlignmentWeightParam->valueChanged.connect(mAlignmentWeightChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::cohesionWeight);
		uni->setValue(mResource->mCohesionWeightParam->mValue);
		mCohesionWeightChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mCohesionWeightParam->valueChanged.connect(mCohesionWeightChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::separationWeight);
		uni->setValue(mResource->mSeparationWeightParam->mValue);
		mSeparationWeightChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mSeparationWeightParam->valueChanged.connect(mSeparationWeightChangedSlot);

		uni = comp_ubo_struct->getOrCreateUniform<UniformFloatInstance>(computeuniform::boundsRadius);
		uni->setValue(mResource->mBoundsRadiusParam->mValue);
		mBoundsRadiusChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mBoundsRadiusParam->valueChanged.connect(mBoundsRadiusChangedSlot);


		// Vertex shader uniforms
		auto* vert_ubo_struct = getMaterialInstance().getOrCreateUniform(uniform::VERTUBO);
		if (vert_ubo_struct == nullptr)
			return false;

		uni = vert_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::boidSize);
		uni->setValue(mResource->mBoidSizeParam->mValue);
		mBoidSizeChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mBoidSizeParam->valueChanged.connect(mBoidSizeChangedSlot);

		uni = vert_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::fresnelScale);
		uni->setValue(mResource->mFresnelScaleParam->mValue);
		mFresnelScaleChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mFresnelScaleParam->valueChanged.connect(mFresnelScaleChangedSlot);

		uni = vert_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::fresnelPower);
		uni->setValue(mResource->mFresnelPowerParam->mValue);
		mFresnelPowerChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mFresnelPowerParam->valueChanged.connect(mFresnelPowerChangedSlot);


		// Fragment shader uniforms
		auto* frag_ubo_struct = getMaterialInstance().getOrCreateUniform(uniform::FRAGUBO);
		if (frag_ubo_struct == nullptr)
			return false;

		auto bool_uni = frag_ubo_struct->getOrCreateUniform<UniformUIntInstance>(uniform::randomColor);
		bool_uni->setValue(mResource->mRandomColorParam->mValue);
		mRandomColorChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformBoolUpdate, this, std::placeholders::_1, bool_uni));
		mResource->mRandomColorParam->valueChanged.connect(mRandomColorChangedSlot);

		auto vec3_uni = frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition);
		vec3_uni->setValue(mResource->mLightPositionParam->mValue);
		mLightPositionChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<glm::vec3>, this, std::placeholders::_1, vec3_uni));
		mResource->mLightPositionParam->valueChanged.connect(mLightPositionChangedSlot);

		uni = frag_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightIntensity);
		uni->setValue(mResource->mLightIntensityParam->mValue);
		mLightIntensityChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mLightIntensityParam->valueChanged.connect(mLightIntensityChangedSlot);

		vec3_uni = frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::diffuseColor);
		vec3_uni->setValue(mResource->mDiffuseColorParam->mValue);
		mDiffuseColorChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, vec3_uni));
		mResource->mDiffuseColorParam->valueChanged.connect(mDiffuseColorChangedSlot);

		vec3_uni = frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightColor);
		vec3_uni->setValue(mResource->mLightColorParam->mValue);
		mLightColorChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, vec3_uni));
		mResource->mLightColorParam->valueChanged.connect(mLightColorChangedSlot);

		vec3_uni = frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::haloColor);
		vec3_uni->setValue(mResource->mHaloColorParam->mValue);
		mHaloColorChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, vec3_uni));
		mResource->mHaloColorParam->valueChanged.connect(mHaloColorChangedSlot);

		vec3_uni = frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::specularColor);
		vec3_uni->setValue(mResource->mSpecularColorParam->mValue);
		mSpecularColorChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, vec3_uni));
		mResource->mSpecularColorParam->valueChanged.connect(mSpecularColorChangedSlot);

		uni = frag_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::shininess);
		uni->setValue(mResource->mShininessParam->mValue);
		mShininessChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mShininessParam->valueChanged.connect(mShininessChangedSlot);

		uni = frag_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::diffuseIntensity);
		uni->setValue(mResource->mDiffuseIntensityParam->mValue);
		mDiffuseIntensityChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mDiffuseIntensityParam->valueChanged.connect(mDiffuseIntensityChangedSlot);

		uni = frag_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::specularIntensity);
		uni->setValue(mResource->mSpecularIntensityParam->mValue);
		mSpecularIntensityChangedSlot.setFunction(std::bind(&FlockingSystemComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, uni));
		mResource->mSpecularIntensityParam->valueChanged.connect(mSpecularIntensityChangedSlot);

		return true;
	}


	void FlockingSystemComponentInstance::update(double deltaTime)
	{
		// Update time variables
		mDeltaTime = deltaTime;
		mElapsedTime += deltaTime;

		// Update time parameters
		mDeltaTimeParam->setValue(static_cast<float>(mDeltaTime));
		mElapsedTimeParam->setValue(static_cast<float>(mElapsedTime));
	}


	void FlockingSystemComponentInstance::compute()
	{
		// Swap buffers
		mBindingIn->setBuffer(*mBoidBufferInput);
		mBindingOut->setBuffer(*mBoidBufferOutput);

		// Update the compute material uniforms of the compute instance
		std::vector<glm::vec3> targets;
		targets.reserve(mTargetTransforms.size());

		uint count = std::min<uint>(mTargetTransforms.size(), mTargetsUniform->getNumElements());
		for (uint i = 0; i < count; i++)
			targets.emplace_back(mTargetTransforms[i]->getGlobalTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		mTargetsUniform->setValues(targets);
		mTargetCountUniform->setValue(targets.size());

		// Compute the compute instance
		// This updates the boid storage buffers to use for rendering
		mRenderService->computeObjects({ mComputeInstance });

		// Swap buffers
		std::swap(mBoidBufferInput, mBoidBufferOutput);
	}


	/**
	 * This onDraw override is almost identical to the default in nap::RenderableMeshComponentInstance. The only difference is that we
	 * set the `instanceCount` of `vkCmdDrawIndexed()` equal to the boid count. This will render the specified number of boids in a
	 * single draw call. We can also identify which boid is rendered using the built-in variable `gl_InstanceIndex`, which is used as
	 * a key to fetch the appropriate data from the boid storage buffer.
	 */
	void FlockingSystemComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Update render storage binding
		mRenderStorageBinding->setBuffer(mBindingOut->getBuffer());

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransformComponent->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

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

		// Make use of instanced rendering by setting the `instanceCount` of `vkCmdDrawIndexed()` equal to the boid count.
		// This renders the boid mesh `mNumboids` times in a single draw call.
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


	FlockingSystemComponent& FlockingSystemComponentInstance::getResource() const
	{
		return *mResource;
	}


	uint FlockingSystemComponentInstance::getNumBoids() const
	{
		return mNumBoids;
	}
}
