/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rendercomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <emptymesh.h>
#include <parameternumeric.h>

namespace nap
{
	// Forward Declares
	class RenderDOFComponentInstance;

	/**
	 * Pre- or post-processing effect that applies a depth-of-field effect to the input texture and renders it to the
	 * specified output render texture. This component manages a custom material based on nap::DOFShader, nap::NoMesh
	 * and nap::RenderTarget internally.
	 *
	 * Resource-part of RenderDOFComponentInstance.
	 */
	class NAPAPI RenderDOFComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderDOFComponent, RenderDOFComponentInstance)
	public:
		ComponentPtr<PerspCameraComponent>	mCamera;						///< Property: 'Camera'
		ResourcePtr<RenderTarget>			mInputTarget;					///< Property: 'InputTarget' the input color target, must be copyable
		ResourcePtr<RenderTexture2D>		mOutputTexture;					///< Property: 'OutputTexture' the output color texture

		ResourcePtr<ParameterFloat>			mAperture;
		ResourcePtr<ParameterFloat>			mFocalLength;
		ResourcePtr<ParameterFloat>			mFocusDistance;
		ResourcePtr<ParameterFloat>			mFocusPower;
	};


	/**
	 * Pre- or post-processing effect that applies a depth-of-field effect to the input texture and renders it to the
	 * specified output render texture. This component manages a custom material based on nap::DOFShader, nap::NoMesh
	 * and nap::RenderTarget internally.
	 *
	 * Simply declare the component in json and call RenderDOFComponentInstance::draw() in the render part of your
	 * application, in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
	 */
	class NAPAPI RenderDOFComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderDOFComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderDOFComponentInstance based on the RenderDOFComponent resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderDOFComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Renders the effect to the output texture, without having to define a render target or mesh.
		 * Call this in your application render() call inbetween nap::RenderService::beginHeadlessRecording()
		 * and nap::RenderService::endHeadlessRecording().
		 * Do not call this function outside of a headless recording pass i.e. when rendering to a window.
		 * Alternatively, you can use the render service to render this component, see onDraw().
		 */
		void draw();

		/**
		 * Returns the output texture that includes the dof effect.
		 * @return the output texture
		 */
		Texture2D& getOutputTexture() { return *mResource->mOutputTexture; }

	protected:
		/**
		 * Draws the effect full screen to the currently active render target
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix ignored
		 * @param projectionMatrix ignored
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		/**
		 * Link to camera, used to get near and far clipping plane values
		 */
		ComponentInstancePtr<PerspCameraComponent> mCamera = { this, &RenderDOFComponent::mCamera };

		RenderDOFComponent*			mResource = nullptr;
		RenderService*				mRenderService = nullptr;			///< Render service

		MaterialInstanceResource	mMaterialInstanceResource;			///< Instance of the material, used to override uniforms for this instance
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.

		RenderTexture2D				mIntermediateTexture;				///< Internally managed render texture
		RenderTarget				mRenderTargetA;						///< Internally managed render target
		RenderTarget				mRenderTargetB;						///< Internally managed render target
		RenderableMesh				mRenderableMesh;					///< Mesh / Material combination
		std::unique_ptr<EmptyMesh>	mEmptyMesh;							///< Empty mesh

		Sampler2DInstance*			mColorTextureSampler = nullptr;		///< Sampler instance for color textures in the blur material
		Sampler2DInstance*			mDepthTextureSampler = nullptr;		///< Sampler instance for depth textures in the blur material
		UniformVec2Instance*		mTextureSizeUniform = nullptr;		///< Texture size uniform of the blur material
		UniformVec2Instance*		mNearFarUniform = nullptr;			///< Near and far clipping plane values of camera
		UniformVec2Instance*		mDirectionUniform = nullptr;		///< Blur direction
		UniformFloatInstance*		mApertureUniform = nullptr;			///< Aperture
		UniformFloatInstance*		mFocalLengthUniform = nullptr;		///< Focal Length
		UniformFloatInstance*		mFocusDistanceUniform = nullptr;	///< Focus Distance
		UniformFloatInstance*		mFocusPowerUniform = nullptr;		///< Pocus Power
	};
}
