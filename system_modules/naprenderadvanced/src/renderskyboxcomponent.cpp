/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderskyboxcomponent.h"
#include "skyboxshader.h"

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

RTTI_BEGIN_CLASS(nap::RenderSkyBoxComponent)
	RTTI_PROPERTY("BlendMode",		&nap::RenderSkyBoxComponent::mBlendMode,	nap::rtti::EPropertyMetaData::Default)		
	RTTI_PROPERTY("CubeTexture",	&nap::RenderSkyBoxComponent::mCubeTexture,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color",			&nap::RenderSkyBoxComponent::mColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Opacity",		&nap::RenderSkyBoxComponent::mBlendMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderSkyBoxComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Checks if the uniform of type T is available on the source material and creates it if so
	 * @param uniformName name of the uniform to create
	 * @param structBuffer the struct that holds the uniform
	 * @param error contains the error if uniform isn't available
	 */
	template<typename T>
	static T* getUniform(const std::string& uniformName, UniformStructInstance& structBuffer, utility::ErrorState& error)
	{
		auto* found_uniform = structBuffer.getOrCreateUniform<T>(uniformName);
		return error.check(found_uniform != nullptr,
			"Unable to get or create uniform with name: %s for struct: %s", uniformName.c_str(), structBuffer.getDeclaration().mName.c_str()) ?
			found_uniform : nullptr;
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderSkyBoxComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderSkyBoxComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}



	//////////////////////////////////////////////////////////////////////////
	// RenderSkyBoxComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderSkyBoxComponentInstance::RenderSkyBoxComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(*entity.getCore()->getService<RenderService>()),
		mSkyBoxMesh(*entity.getCore())
	{ }


	bool RenderSkyBoxComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<RenderSkyBoxComponent>();

		// Initialize base renderable mesh component instance
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get transform
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr,
			"%s: Missing transform component", mResource->mID.c_str()))
			return false;

		// Initialize skybox mesh
		mSkyBoxMesh.mPolygonMode = EPolygonMode::Fill;
		mSkyBoxMesh.mUsage = EMemoryUsage::Static;
		mSkyBoxMesh.mCullMode = ECullMode::Front;
		mSkyBoxMesh.mFlipNormals = false;
		if (!mSkyBoxMesh.init(errorState))
			return false;

		// Get or create skybox material
		auto* skybox_material = mRenderService.getOrCreateMaterial<SkyBoxShader>(errorState);
		if (!errorState.check(skybox_material != nullptr,
			"%s: unable to get or create skybox material", mResource->mID.c_str()))
			return false;

		// Setup and create skybox material instance
		mMaterialInstanceResource.mBlendMode = mResource->mBlendMode;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = skybox_material;

		if (!mMaterialInstance.init(mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		// Ensure the mvp struct is available
		auto* mvp_struct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mvp_struct != nullptr, "%s: Unable to find uniform struct: %s in shader: %s",
			this->mID.c_str(), uniform::mvpStruct, RTTI_OF(SkyBoxShader).get_name().data()))
			return false;

		// Get all matrices
		mModelMatUniform = getUniform<UniformMat4Instance>(uniform::modelMatrix, *mvp_struct, errorState);
		mProjectMatUniform = getUniform<UniformMat4Instance>(uniform::projectionMatrix, *mvp_struct, errorState);
		mViewMatUniform = getUniform<UniformMat4Instance>(uniform::viewMatrix, *mvp_struct, errorState);

		if (mModelMatUniform == nullptr || mProjectMatUniform == nullptr || mViewMatUniform == nullptr)
			return false;

		// Ensure ubo is available
		auto* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::skybox::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find struct: %s in shader: %s",
			this->mID.c_str(), uniform::skybox::uboStruct, RTTI_OF(SkyBoxShader).get_name().data()))
			return false;

		// Get ubo uniforms
		mAlphaUniform = getUniform<UniformFloatInstance>(uniform::skybox::alpha, *ubo_struct, errorState);
		mColorUniform = getUniform<UniformVec3Instance>(uniform::skybox::color, *ubo_struct, errorState);

		if (mAlphaUniform == nullptr || mColorUniform == nullptr)
			return false;

		// Get cube sampler binding and set
		mCubeSampler = mMaterialInstance.getOrCreateSampler<SamplerCubeInstance>(uniform::skybox::cubeTexture);
		if (!errorState.check(mCubeSampler != nullptr,
			"%s: Missing cube sampler input: %s", mID.c_str(), uniform::skybox::cubeTexture))
			return false;

		// Set uniforms
		mAlphaUniform->setValue(mResource->mOpacity);
		mColorUniform->setValue(mResource->mColor);
		mCubeSampler->setTexture(*mResource->mCubeTexture);

		// Create render-able mesh
		mRenderableMesh = mRenderService.createRenderableMesh(mSkyBoxMesh, mMaterialInstance, errorState);
		return mRenderableMesh.isValid();
	}


	void RenderSkyBoxComponentInstance::setTexture(const TextureCube& texture)
	{
		mCubeSampler->setTexture(const_cast<TextureCube&>(texture));
	}


	void RenderSkyBoxComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		assert(mRenderableMesh.isValid());
		mProjectMatUniform->setValue(projectionMatrix);
		mViewMatUniform->setValue(viewMatrix);
		mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		// Acquire new / unique descriptor set before rendering
		const DescriptorSet& descriptor_set = mMaterialInstance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService.getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

		// Draw meshes
		MeshInstance& mesh_instance = mSkyBoxMesh.getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		const IndexBuffer& index_buffer = mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);
	}
}
