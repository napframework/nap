/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderfadercomponent.h"
#include "fadeshader.h"

// External Includes
#include <renderglobals.h>
#include <material.h>
#include <renderservice.h>
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::RenderFaderComponent)
	RTTI_PROPERTY("FadeIn",				&nap::RenderFaderComponent::mFadeIn,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FadeDuration",		&nap::RenderFaderComponent::mFadeDuration,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FadeColor",			&nap::RenderFaderComponent::mFadeColor,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderFaderComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderFaderComponentInstance
	//////////////////////////////////////////////////////////////////////////
		
	RenderFaderComponentInstance::RenderFaderComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<nap::RenderService>()),
		mEmptyMesh(*entity.getCore())
	{ }


	bool RenderFaderComponentInstance::init(utility::ErrorState& errorState)
	{
		// Initialize base
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Initialize material based on resource
		mResource = getComponent<RenderFaderComponent>();

		// Create fade material
		Material* fade_material = mRenderService->getOrCreateMaterial<FadeShader>(errorState);
		if (!errorState.check(fade_material != nullptr, "%s: unable to get or create constant material", mID.c_str()))
			return false;

		// Create resource for the fade material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::AlphaBlend;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = fade_material;

		// Create constant material instance
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Initialize empty mesh
		if (!mEmptyMesh.init(errorState))
			return false;

		// Create mesh / material combo that we can render
		mRenderableMesh = mRenderService->createRenderableMesh(mEmptyMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Get uniforms
		auto uniform_struct = mMaterialInstance.getOrCreateUniform(uniform::fade::uboStruct);
		if (!errorState.check(uniform_struct != nullptr, "%s: Unable to find uniform struct: %s in shader: %s",
			mID.c_str(), uniform::fade::uboStruct, RTTI_OF(FadeShader).get_name().data()))
			return false;

		// Store alpha (updated at runtime)
		mAlphaUniform = uniform_struct->getOrCreateUniform<UniformFloatInstance>(uniform::fade::alpha);
		if (!errorState.check(mAlphaUniform != nullptr, "%s: Unable to find uniform: %s in shader: %s",
			mID.c_str(), uniform::fade::alpha, RTTI_OF(FadeShader).get_name().data()))
			return false;

		// Set color
		auto* color_uniform = uniform_struct->getOrCreateUniform<UniformVec3Instance>(uniform::fade::color);
		if (!errorState.check(color_uniform != nullptr, "%s: Unable to find uniform: %s in shader: %s",
			mID.c_str(), uniform::fade::color, RTTI_OF(FadeShader).get_name().data()))
			return false;
		color_uniform->setValue(mResource->mFadeColor.toVec3());

		// Setup fade state
		mAlphaUniform->setValue(0.0f);
		if (mResource->mFadeIn)
			fadeIn();

		return true;
	}


	void RenderFaderComponentInstance::update(double deltaTime)
	{
		switch (mFadeState)
		{
			case EFadeState::FadingIn:
			{
				double elapsed = mTimer.getElapsedTime();
				if (elapsed > mResource->mFadeDuration)
				{
					mFadeState = EFadeState::FadedIn;
					mFadedIn();
				}
				mAlphaUniform->setValue(1.0 -
					math::smoothStep(static_cast<float>(elapsed), 0.0f, mResource->mFadeDuration));
				break;
			}
			case EFadeState::FadingOut:
			{
				double elapsed = mTimer.getElapsedTime();
				if (elapsed > mResource->mFadeDuration)
				{
					mFadeState = EFadeState::FadedOut;
					mFadedOut();
				}
				mAlphaUniform->setValue(
					math::smoothStep(static_cast<float>(elapsed), 0.0f, mResource->mFadeDuration));
				break;
			}
			default:
			{
				break;
			}
		}
	}


	// Draw Mesh
	void RenderFaderComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Don't draw fade when fade has no effect
		if (mFadeState == EFadeState::FadedIn)
			return;

		// Acquire descriptor set before rendering
		const DescriptorSet& descriptor_set = mMaterialInstance.update();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mEmptyMesh, mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bufferless drawing with the fade shader
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}	
} 
