/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderdofcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"
#include "dofshader.h"
#include "textureutils.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::RenderDOFComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderDOFComponent)
	RTTI_PROPERTY("Camera",						&nap::RenderDOFComponent::mCamera,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("InputTarget",				&nap::RenderDOFComponent::mInputTarget,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputTexture",				&nap::RenderDOFComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Aperture",					&nap::RenderDOFComponent::mAperture,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FocalLength",				&nap::RenderDOFComponent::mFocalLength,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FocusDistance",				&nap::RenderDOFComponent::mFocusDistance,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FocusPower",					&nap::RenderDOFComponent::mFocusPower,					nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::RenderDOFComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderDOFComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	RenderDOFComponentInstance::RenderDOFComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderTargetA(*entity.getCore()),
		mRenderTargetB(*entity.getCore()),
		mIntermediateTexture(*entity.getCore()),
		mEmptyMesh(std::make_unique<EmptyMesh>(*entity.getCore()))
	{ }


	bool RenderDOFComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		mResource = getComponent<RenderDOFComponent>();

		// Verify the input render target is available
		if (!errorState.check(mResource->mInputTarget != nullptr, "InputTarget not set"))
			return false;

		// Verify the render target has a depth texture resource
		if (!errorState.check(mResource->mInputTarget->hasDepthTexture(), "The specified InputTarget has no depth texture resource"))
			return false;

		// Get render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();

		// Create material
		Material* dof_material = mRenderService->getOrCreateMaterial<DOFShader>(errorState);
		if (!errorState.check(dof_material != nullptr, "%s: unable to get or create blur material", mResource->mID.c_str()))
			return false;

		// Create material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = dof_material;
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Get UBO struct
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::dof::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), uniform::dof::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get texture size uniform
		mTextureSizeUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::textureSize);
		if (!errorState.check(mTextureSizeUniform != nullptr, "Missing uniform vec2 'textureSize' in uniform UBO"))
			return false;

		mDirectionUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::direction);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		mNearFarUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::nearFar);
		if (!errorState.check(mNearFarUniform != nullptr, "Missing uniform vec2 'nearFar' in uniform UBO"))
			return false;

		mApertureUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::aperture);
		if (!errorState.check(mApertureUniform != nullptr, "Missing uniform float 'aperture' in uniform UBO"))
			return false;

		mFocalLengthUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::focalLength);
		if (!errorState.check(mFocalLengthUniform != nullptr, "Missing uniform float 'focalLength' in uniform UBO"))
			return false;

		mFocusDistanceUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::focusDistance);
		if (!errorState.check(mFocusDistanceUniform != nullptr, "Missing uniform float 'focusDistance' in uniform UBO"))
			return false;

		mFocusPowerUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::focusPower);
		if (!errorState.check(mFocusPowerUniform != nullptr, "Missing uniform float 'focusPower' in uniform UBO"))
			return false;

		// Get color texture sampler
		mColorTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::dof::sampler::colorTexture);
		if (!errorState.check(mColorTextureSampler != nullptr, "Missing uniform sampler2D 'colorTexture'"))
			return false;

		// Get depth texture sampler
		mDepthTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::dof::sampler::depthTexture);
		if (!errorState.check(mDepthTextureSampler != nullptr, "Missing uniform sampler2D 'depthTexture'"))
			return false;

		// Create no mesh
		if (!mEmptyMesh->init(errorState))
			return false;

		mRenderableMesh = mRenderService->createRenderableMesh(*mEmptyMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Create intermediate texture
		mIntermediateTexture.mID = utility::stringFormat("%s_DOF_%s", RTTI_OF(RenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
		mIntermediateTexture.mWidth = mResource->mInputTarget->getBufferSize().x;
		mIntermediateTexture.mHeight = mResource->mInputTarget->getBufferSize().y;
		mIntermediateTexture.mColorFormat = mResource->mInputTarget->getColorTexture().mColorFormat;
		mIntermediateTexture.mUsage = Texture::EUsage::Static;
		if (!mIntermediateTexture.init(errorState))
		{
			errorState.fail("%s: Failed to initialize internal render target", mIntermediateTexture.mID.c_str());
			return false;
		}

		// Create render targets
		mRenderTargetA.mColorTexture = &mIntermediateTexture;
		mRenderTargetB.mColorTexture = mResource->mOutputTexture;
		std::vector<RenderTarget*> targets = { &mRenderTargetA, &mRenderTargetB };
		for (uint i = 0; i< targets.size(); i++)
		{
			auto* rt = targets[i];
			rt->mID = utility::stringFormat("%s_DOF_%s", RTTI_OF(RenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());	
			rt->mClearColor = mResource->mInputTarget->getClearColor();
			rt->mSampleShading = false;
			rt->mRequestedSamples = ERasterizationSamples::One;
			if (!rt->init(errorState))
			{
				errorState.fail("%s: Failed to initialize internal render target", rt->mID.c_str());
				return false;
			}
		}

		return true;
	}


	void RenderDOFComponentInstance::draw()
	{
		mColorTextureSampler->setTexture(mResource->mInputTarget->getColorTexture());
		mDepthTextureSampler->setTexture(mResource->mInputTarget->getDepthTexture());
		mTextureSizeUniform->setValue(mResource->mInputTarget->getColorTexture().getSize());
		mNearFarUniform->setValue({ mCamera->getNearClippingPlane(), mCamera->getFarClippingPlane() });
		mApertureUniform->setValue(mResource->mAperture->mValue);
		mFocalLengthUniform->setValue(mResource->mFocalLength->mValue);
		mFocusPowerUniform->setValue(mResource->mFocusPower->mValue);
		mFocusDistanceUniform->setValue(mResource->mFocusDistance->mValue);
		mDirectionUniform->setValue({1.0f, 0.0f});

		// Horizontal pass
		mRenderTargetA.beginRendering();
		onDraw(mRenderTargetA, mRenderService->getCurrentCommandBuffer(), {}, {});
		mRenderTargetA.endRendering();

		// Vertical pass
		mColorTextureSampler->setTexture(mIntermediateTexture);
		mDirectionUniform->setValue({ 0.0f, 1.0f });

		mRenderTargetB.beginRendering();
		onDraw(mRenderTargetB, mRenderService->getCurrentCommandBuffer(), {}, {});
		mRenderTargetB.endRendering();
	}


	void RenderDOFComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		const DescriptorSet& descriptor_set = mMaterialInstance.update();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}
}
