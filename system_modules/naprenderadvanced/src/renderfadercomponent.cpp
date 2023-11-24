/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderfadercomponent.h"
#include "fadeshader.h"

// External Includes
#include <mesh.h>
#include <renderglobals.h>
#include <material.h>
#include <renderservice.h>
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::RenderFaderComponent)
	RTTI_PROPERTY("FadeDuration",		&nap::RenderFaderComponent::mFadeDuration,					nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Camera",				&nap::RenderFaderComponent::mCamera,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderFaderComponent::mMaterialInstanceResource,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StartBlack",			&nap::RenderFaderComponent::mStartBlack,					nap::rtti::EPropertyMetaData::Default)
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
		mRenderService(entity.getCore()->getService<nap::RenderService>())
	{ }


	bool RenderFaderComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Initialize material based on resource
		mResource = getComponent<RenderFaderComponent>();
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), mResource->mMaterialInstanceResource, errorState))
			return false;

		// Create no mesh
		mNoMesh = std::make_unique<NoMesh>(*getEntityInstance()->getCore());
		if (!mNoMesh->init(errorState))
			return false;

		mRenderableMesh = createRenderableMesh(*mNoMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		mElapsedTime = 0.0f;
		mFadeState = mResource->mStartBlack ? EFadeState::Out : EFadeState::Off;

		return true;
	}


	void RenderFaderComponentInstance::update(double deltaTime)
	{
		mElapsedTime += static_cast<float>(deltaTime);

		// Update uniforms
		auto* ubo = getMaterialInstance().getOrCreateUniform("UBO");
		if (ubo != nullptr)
		{
			float transition_value = std::max(mElapsedTime - mFadeStartTime, 0.0f) / mResource->mFadeDuration->getValue();

			switch (mFadeState)
			{
				case EFadeState::In:
				{
					float opacity = std::clamp(transition_value, 0.0f, 1.0f);
					ubo->getOrCreateUniform<UniformFloatInstance>(uniform::fade::alpha)->setValue(opacity);

					// Trigger fade to black signal
					if (transition_value >= 1.0f)
					{
						mFadeState = EFadeState::Out;
						mFadeStartTime = mElapsedTime;
						mFadedToBlackSignal();
					}
					break;
				}
				case EFadeState::Out:
				{
					float opacity = 1.0f - std::clamp(transition_value, 0.0f, 1.0f);
					ubo->getOrCreateUniform<UniformFloatInstance>(uniform::fade::alpha)->setValue(opacity);

					// Trigger fade to black signal
					if (transition_value >= 1.0f)
					{
						mFadeState = EFadeState::Off;
						mFadedOutSignal();
					}
					break;
				}
				default:
				{
					ubo->getOrCreateUniform<UniformFloatInstance>(uniform::fade::alpha)->setValue(0.0f);
				}
			}
		}
	}


	void RenderFaderComponentInstance::startFade()
	{
		mFadeState = EFadeState::In;
		mFadeStartTime = mElapsedTime;
	}


	RenderableMesh RenderFaderComponentInstance::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		return render_service->createRenderableMesh(mesh, materialInstance, errorState);
	}


	RenderableMesh RenderFaderComponentInstance::createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState)
	{
		return createRenderableMesh(mesh, mMaterialInstance, errorState);
	}


	void RenderFaderComponentInstance::setMesh(const RenderableMesh& mesh)
	{
		assert(mesh.isValid());
		mRenderableMesh = mesh;
	}


	// Draw Mesh
	void RenderFaderComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Do not render if no fade is active
		if (!isFadeActive())
			return;

		// Acquire descriptor set before rendering
		const DescriptorSet& descriptor_set = getMaterialInstance().update();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, *mNoMesh, mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bufferless drawing with the fade shader
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}


	bool RenderFaderComponentInstance::isSupported(CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(PerspCameraComponentInstance));
	}


	bool RenderFaderComponentInstance::isFadeActive() const
	{
		return mFadeState != EFadeState::Off;
	}
} 
