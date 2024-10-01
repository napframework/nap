/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <renderablemeshcomponent.h>
#include <uniforminstance.h>
#include <audioroadcomponent.h>

namespace nap
{
	class RenderAudioRoadComponentInstance;

	/**
	 * Renders a `nap::AudioRoadComponent` with the specified Mesh. Offers handy parameter mappings for material properties.
	 * The position and normal vertex buffers are pulled from the audio road component.
	 */
	class NAPAPI RenderAudioRoadComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderAudioRoadComponent, RenderAudioRoadComponentInstance)
	public:
		ComponentPtr<AudioRoadComponent> mAudioRoadComponent;

		ResourcePtr<ParameterRGBColorFloat>		mAmbient;
		ResourcePtr<ParameterRGBColorFloat>		mDiffuse;
		ResourcePtr<ParameterRGBColorFloat>		mSpecular;
		ResourcePtr<ParameterRGBColorFloat>		mHighlight;
		ResourcePtr<ParameterRGBColorFloat>		mFresnelColor;
		ResourcePtr<ParameterVec2>				mFresnel;
		ResourcePtr<ParameterFloat>				mShininess;
		ResourcePtr<ParameterFloat>				mReflection;
		ResourcePtr<ParameterFloat>				mHighlightLength;
		ResourcePtr<ParameterFloat>				mAlpha;
		ResourcePtr<ParameterBool>				mEnvironment;
	};


	/**
	 * Renders a `nap::AudioRoadComponent` with the specified Mesh. Offers handy parameter mappings for material properties.
	 * The position and normal vertex buffers are pulled from the audio road component.
	 */
	class NAPAPI RenderAudioRoadComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		RenderAudioRoadComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableMeshComponentInstance(entity, resource) { }

		/**
		 * Initialize RenderAudioRoadComponentInstance based on the RenderAudioRoadComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderAudioRoadComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		ComponentInstancePtr<AudioRoadComponent> mAudioRoadComponent = { this, &RenderAudioRoadComponent::mAudioRoadComponent };

	private:
		RenderAudioRoadComponent* mResource = nullptr;

		// Update handlers
		template<typename T>
		void onUniformValueUpdate(T value, TypedUniformValueInstance<T>* uniformInstance)
		{
			assert(uniformInstance != nullptr);
			uniformInstance->setValue(value);
		}

		void onUniformRGBColorUpdate(RGBColorFloat value, UniformVec3Instance* uniformInstance)
		{
			assert(uniformInstance != nullptr);
			uniformInstance->setValue(value.toVec3());
		}

		void onUniformBoolUpdate(bool value, UniformUIntInstance* uniformInstance)
		{
			assert(uniformInstance != nullptr);
			uniformInstance->setValue(value);
		}

		// Slots
		Slot<RGBColorFloat>		mAmbientChangedSlot;
		Slot<RGBColorFloat>		mDiffuseChangedSlot;
		Slot<RGBColorFloat>		mSpecularChangedSlot;
		Slot<RGBColorFloat>		mHighlightChangedSlot;
		Slot<RGBColorFloat>		mFresnelColorChangedSlot;
		Slot<glm::vec2>			mFresnelChangedSlot;
		Slot<float>				mShininessChangedSlot;
		Slot<float>				mReflectionChangedSlot;
		Slot<float>				mHighlightLengthChangedSlot;
		Slot<float>				mAlphaChangedSlot;
		Slot<bool>				mEnvironmentChangedSlot;
	};
}
