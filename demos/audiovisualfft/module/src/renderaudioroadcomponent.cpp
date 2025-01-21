/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderaudioroadcomponent.h"

// External Includes
#include <entity.h>
#include <computecomponent.h>
#include <renderglobals.h>
#include <mesh.h>

// nap::RenderAudioRoadComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderAudioRoadComponent)
	RTTI_PROPERTY("Mesh", 						&nap::RenderAudioRoadComponent::mMesh, 						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AudioRoadComponent", 		&nap::RenderAudioRoadComponent::mAudioRoad, 				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstanceResource", 	&nap::RenderAudioRoadComponent::mMaterialInstanceResource, 	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClipRect", 					&nap::RenderAudioRoadComponent::mClipRect, 					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LineWidth", 					&nap::RenderAudioRoadComponent::mLineWidth, 				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Ambient",					&nap::RenderAudioRoadComponent::mAmbient,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Diffuse",					&nap::RenderAudioRoadComponent::mDiffuse,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Specular",					&nap::RenderAudioRoadComponent::mSpecular,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FresnelColor",				&nap::RenderAudioRoadComponent::mFresnelColor,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Fresnel",					&nap::RenderAudioRoadComponent::mFresnel,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Shininess",					&nap::RenderAudioRoadComponent::mShininess,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Alpha",						&nap::RenderAudioRoadComponent::mAlpha,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Highlight",					&nap::RenderAudioRoadComponent::mHighlight,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighlightLength",			&nap::RenderAudioRoadComponent::mHighlightLength,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Reflection",					&nap::RenderAudioRoadComponent::mReflection,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Environment",				&nap::RenderAudioRoadComponent::mEnvironment,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::RenderAudioRoadComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAudioRoadComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION(nap::material::instance::getOrCreateMaterial, &nap::RenderAudioRoadComponentInstance::getOrCreateMaterial)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool RenderAudioRoadComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Fetch resource
		mResource = getComponent<RenderAudioRoadComponent>();

		// Get vec4 plane mesh
		mMesh = mResource->mMesh.get();

		if (!mMaterialInstance.init(*mRenderService, mResource->mMaterialInstanceResource, errorState))
			return false;

		mRenderableMesh = mRenderService->createRenderableMesh(*mMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Ensure there is a transform component
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = mResource->mClipRect;

		// Copy line width, ensure it's supported
		mLineWidth = mResource->mLineWidth;
		if (mLineWidth > 1.0f && !mRenderService->getWideLinesSupported())
		{
			nap::Logger::warn("Unsupported line width: %.02f", mLineWidth);
			mLineWidth = 1.0f;
		}

		// Cache MVP Uniforms
		auto* mvp = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp != nullptr)
		{
			mModelMatUniform 		= mvp->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mViewMatUniform 		= mvp->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mProjectMatUniform 		= mvp->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
			mNormalMatrixUniform 	= mvp->getOrCreateUniform<UniformMat4Instance>(uniform::normalMatrix);
			mCameraWorldPosUniform 	= mvp->getOrCreateUniform<UniformVec3Instance>(uniform::cameraPosition);
		}

		/**
		 * Assume BlinnPhongShader material interface
		 */
		auto* uni = mMaterialInstance.getOrCreateUniform("UBO");
		if (!errorState.check(uni != nullptr, "Missing uniform struct with name `UBO`"))
			return false;

		auto* ambient = uni->getOrCreateUniform<UniformVec3Instance>("ambient");
		if (ambient != nullptr && mResource->mAmbient != nullptr)
		{
			ambient->setValue(mResource->mAmbient->mValue);
			mAmbientChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, ambient));
			mResource->mAmbient->valueChanged.connect(mAmbientChangedSlot);
		}

		auto* diffuse = uni->getOrCreateUniform<UniformVec3Instance>("diffuse");
		if (diffuse != nullptr && mResource->mDiffuse != nullptr)
		{
			diffuse->setValue(mResource->mDiffuse->mValue);
			mDiffuseChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, diffuse));
			mResource->mDiffuse->valueChanged.connect(mDiffuseChangedSlot);
		}

		auto* specular = uni->getOrCreateUniform<UniformVec3Instance>("specular");
		if (specular != nullptr && mResource->mSpecular != nullptr)
		{
			specular->setValue(mResource->mSpecular->mValue);
			mSpecularChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, specular));
			mResource->mSpecular->valueChanged.connect(mSpecularChangedSlot);
		}

		auto* highlight = uni->getOrCreateUniform<UniformVec3Instance>("highlight");
		if (highlight != nullptr && mResource->mHighlight != nullptr)
		{
			highlight->setValue(mResource->mHighlight->mValue);
			mHighlightChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, highlight));
			mResource->mHighlight->valueChanged.connect(mHighlightChangedSlot);
		}

		auto* fresnel_color = uni->getOrCreateUniform<UniformVec3Instance>("fresnelColor");
		if (fresnel_color != nullptr && mResource->mFresnelColor != nullptr)
		{
			fresnel_color->setValue(mResource->mFresnelColor->mValue);
			mFresnelColorChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, fresnel_color));
			mResource->mFresnelColor->valueChanged.connect(mFresnelColorChangedSlot);
		}

		auto* fresnel = uni->getOrCreateUniform<UniformVec2Instance>("fresnel");
		if (fresnel != nullptr && mResource->mFresnel != nullptr)
		{
			fresnel->setValue(mResource->mFresnel->mValue);
			mFresnelChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformValueUpdate<glm::vec2>, this, std::placeholders::_1, fresnel));
			mResource->mFresnel->valueChanged.connect(mFresnelChangedSlot);
		}

		auto* shininess = uni->getOrCreateUniform<UniformFloatInstance>("shininess");
		if (shininess != nullptr && mResource->mShininess != nullptr)
		{
			shininess->setValue(mResource->mShininess->mValue);
			mShininessChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, shininess));
			mResource->mShininess->valueChanged.connect(mShininessChangedSlot);
		}

		auto* reflection = uni->getOrCreateUniform<UniformFloatInstance>("reflection");
		if (reflection != nullptr && mResource->mReflection != nullptr)
		{
			reflection->setValue(mResource->mReflection->mValue);
			mReflectionChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, reflection));
			mResource->mReflection->valueChanged.connect(mReflectionChangedSlot);
		}

		auto* highlight_length = uni->getOrCreateUniform<UniformFloatInstance>("highlightLength");
		if (highlight_length != nullptr && mResource->mHighlightLength != nullptr)
		{
			highlight_length->setValue(mResource->mHighlightLength->mValue);
			mHighlightLengthChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, highlight_length));
			mResource->mHighlightLength->valueChanged.connect(mHighlightLengthChangedSlot);
		}

		auto* alpha = uni->getOrCreateUniform<UniformFloatInstance>("alpha");
		if (alpha != nullptr && mResource->mAlpha != nullptr)
		{
			alpha->setValue(mResource->mAlpha->mValue);
			mAlphaChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, alpha));
			mResource->mAlpha->valueChanged.connect(mAlphaChangedSlot);
		}

		auto* environment = uni->getOrCreateUniform<UniformUIntInstance>("environment");
		if (environment != nullptr && mResource->mEnvironment != nullptr)
		{
			environment->setValue(mResource->mEnvironment->mValue);
			mEnvironmentChangedSlot.setFunction(std::bind(&RenderAudioRoadComponentInstance::onUniformBoolUpdate, this, std::placeholders::_1, environment));
			mResource->mEnvironment->valueChanged.connect(mEnvironmentChangedSlot);
		}

		return true;
	}


	void RenderAudioRoadComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
			mModelMatUniform->setValue(mTransform->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransform->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		// Acquire new / unique descriptor set before rendering
		const auto& descriptor_set = mMaterialInstance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		{
			// Copy the ordered vector of VkBuffers from the renderable mesh
			auto vertex_buffers = mRenderableMesh.getVertexBuffers();

			// Override position vertex attribute buffer with storage buffer.
			// We do this by first fetching the internal buffer binding index of the position vertex attribute.
			int position_attr_binding_idx = mRenderableMesh.getVertexBufferBindingIndex(vertexid::position);
			if (position_attr_binding_idx >= 0)
			{
				// Overwrite the VkBuffer under the previously fetched position vertex attribute index.
				vertex_buffers[position_attr_binding_idx] = mAudioRoadComponent->getPositionBuffer().getBuffer();
			}

			// Repeat for the normal attribute
			int normal_attr_binding_idx = mRenderableMesh.getVertexBufferBindingIndex(vertexid::normal);
			if (normal_attr_binding_idx >= 0)
				vertex_buffers[normal_attr_binding_idx] = mAudioRoadComponent->getNormalBuffer().getBuffer();

			// Get offsets
			const auto& offsets = mRenderableMesh.getVertexBufferOffsets();

			// Bind buffers
			// The shader will now use the storage buffer updated by the compute shader as a vertex buffer when rendering the current mesh.
			vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
		}

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
		const auto& mesh_instance = mMesh->getMeshInstance();
		const auto& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const auto& index_buffer = mesh.getIndexBuffer(index);
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
