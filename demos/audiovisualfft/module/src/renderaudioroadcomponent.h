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
	class NAPAPI RenderAudioRoadComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderAudioRoadComponent, RenderAudioRoadComponentInstance)
	public:
		ResourcePtr<PlaneMeshVec4>				mMesh;								///< Property: 'Mesh' the plane mesh to render
		ComponentPtr<AudioRoadComponent> 		mAudioRoad;							///< Property: 'AudioRoad' the audio road that computes the positions and normals
		MaterialInstanceResource				mMaterialInstanceResource;			///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		math::Rect								mClipRect;							///< Property: 'ClipRect' Optional clipping rectangle, in pixel coordinates
		float									mLineWidth = 1.0f;					///< Property: 'LineWidth' Width of the line when rendered, values higher than 1.0 only work when the GPU supports it

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
	class NAPAPI RenderAudioRoadComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderAudioRoadComponentInstance(EntityInstance& entity, Component& resource) :
				RenderableComponentInstance(entity, resource) { }

		/**
		 * Initialize RenderAudioRoadComponentInstance
		 * @param errorState error if initialization fails
		 * @return if the RenderAudioRoadComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Renders the road
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Returns the shader interface used to render the road.
		 * @return material handle
		 */
		MaterialInstance* getOrCreateMaterial() { return &mMaterialInstance; }

		ComponentInstancePtr<AudioRoadComponent> mAudioRoadComponent = { this, &RenderAudioRoadComponent::mAudioRoad };

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

		PlaneMeshVec4*				mMesh = nullptr;					///< The plane mesh to render
		TransformComponentInstance*	mTransform = nullptr;				///< Cached pointer to transform
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.
		math::Rect					mClipRect;							///< Clipping rectangle for this instance, in pixel coordinates
		RenderableMesh				mRenderableMesh;					///< The currently active renderable mesh, either set during init() or set by setMesh

		UniformMat4Instance*		mModelMatUniform = nullptr;			///< Pointer to the model matrix uniform
		UniformMat4Instance*		mViewMatUniform = nullptr;			///< Pointer to the view matrix uniform
		UniformMat4Instance*		mProjectMatUniform = nullptr;		///< Pointer to the projection matrix uniform
		UniformMat4Instance*		mNormalMatrixUniform = nullptr;		///< Pointer to the normal matrix uniform
		UniformVec3Instance*		mCameraWorldPosUniform = nullptr;	///< Pointer to the camera world position unifo
		float						mLineWidth = 1.0f;					///< Line width, clamped to 1.0 if not supported by GPU

		// Slots
		Slot<RGBColorFloat>			mAmbientChangedSlot;
		Slot<RGBColorFloat>			mDiffuseChangedSlot;
		Slot<RGBColorFloat>			mSpecularChangedSlot;
		Slot<RGBColorFloat>			mHighlightChangedSlot;
		Slot<RGBColorFloat>			mFresnelColorChangedSlot;
		Slot<glm::vec2>				mFresnelChangedSlot;
		Slot<float>					mShininessChangedSlot;
		Slot<float>					mReflectionChangedSlot;
		Slot<float>					mHighlightLengthChangedSlot;
		Slot<float>					mAlphaChangedSlot;
		Slot<bool>					mEnvironmentChangedSlot;
	};
}
