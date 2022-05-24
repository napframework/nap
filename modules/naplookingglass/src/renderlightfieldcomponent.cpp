/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderlightfieldcomponent.h"

// Local Includes
#include "lightfieldshader.h"
#include "quiltrendertarget.h"
#include "quiltsettings.h"

// External Includes
#include <entity.h>
#include <rendertarget.h>
#include <renderservice.h>
#include <uniforminstance.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::RenderLightFieldComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderLightFieldComponent)
	RTTI_PROPERTY("QuiltTexture",				&nap::RenderLightFieldComponent::mQuiltTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LookingGlassDevice",			&nap::RenderLightFieldComponent::mDevice,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OverScan",					&nap::RenderLightFieldComponent::mOverscan,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InvertQuilt",				&nap::RenderLightFieldComponent::mInvertQuilt,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Debug",						&nap::RenderLightFieldComponent::mDebug,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::RenderLightFieldComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderLightFieldComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		constexpr const char* UBO = "UBO";
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderLightFieldComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderLightFieldComponentInstance::RenderLightFieldComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mDummyMesh(*entity.getCore())
	{ }


	bool RenderLightFieldComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		RenderLightFieldComponent* resource = getComponent<RenderLightFieldComponent>();

		// Get reference to input texture
		mQuiltTexture = resource->mQuiltTexture.get();

		// Get render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();

		// Create material
		Material* lightfield_material = mRenderService->getOrCreateMaterial<LightFieldShader>(errorState);		
		if (!errorState.check(lightfield_material != nullptr, "%s: unable to get or create blur material", resource->mID.c_str()))
			return false;

		// Create material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = lightfield_material;
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Now create a dummy mesh and initialize it
		if (!mDummyMesh.init(errorState))
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mDummyMesh, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Get UBO struct
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::lightfield::UBO);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), uniform::lightfield::UBO, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Calibration
		const auto& calibration = resource->mDevice->getCalibrationSettings();

		// Quilt
		const auto& settings = resource->mDevice->getQuiltSettings();

		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::pitch)->setValue(calibration.pitch);
		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::tilt)->setValue(calibration.tilt);
		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::center)->setValue(calibration.center);
		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::invView)->setValue(calibration.invView);
		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::subp)->setValue(calibration.subp);
		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::displayAspect)->setValue(calibration.displayAspect);
		ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightfield::quiltAspect)->setValue(calibration.displayAspect);
		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::ri)->setValue(calibration.ri);
		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::bi)->setValue(calibration.bi);

		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::overscan)->setValue(resource->mOverscan ? 1 : 0);
		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::quiltInvert)->setValue(resource->mInvertQuilt ? 1 : 0);
		ubo_struct->getOrCreateUniform<UniformIntInstance>(uniform::lightfield::debug)->setValue(resource->mDebug ? 1 : 0);

		int view_width = settings.mWidth / settings.mColumns;
		int view_height = settings.mHeight / settings.mRows;

		glm::vec3 tile = glm::vec3(settings.mColumns, settings.mRows, settings.getViewCount());
		glm::vec2 view_portion = glm::vec2(float(view_width * settings.mColumns) / float(settings.mWidth), float(view_height * settings.mRows) / float(settings.mHeight));
		float aspect = settings.mWidth / static_cast<float>(settings.mHeight);

		ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightfield::tile)->setValue(tile);
		ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::lightfield::viewPortion)->setValue(view_portion);

		// Texture
		auto* sampler = rtti_cast<Sampler2DInstance>(mMaterialInstance.getOrCreateSampler(sampler::lightfield::screenTex));
		sampler->setTexture(*resource->mQuiltTexture);

		return true;
	}


	void RenderLightFieldComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get valid descriptor set
		VkDescriptorSet descriptor_set = mMaterialInstance.update();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

		// Call vertex shader three times to form a triangle, without buffers.
		// UV coordinates for sampling are generated inside vertex shader using builtin gl_VertexIndex variable.
		vkCmdDraw(mRenderService->getCurrentCommandBuffer(), 3, 1, 0, 0);
	}


	bool RenderLightFieldComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}
}
