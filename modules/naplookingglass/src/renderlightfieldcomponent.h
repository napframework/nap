/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "lookingglassdevice.h"
#include "dummymesh.h"
#include "quiltsettings.h"

// External includes
#include <rendercomponent.h>

namespace nap
{
	// Forward Declares
	class RenderLightFieldComponentInstance;

	/**
	 * Resource-part of RenderLightFieldComponentInstance.
	 *
	 * Applies `nap::LightFieldShader` to the specified quilt `QuiltTexture`. The shader is fully calibrated
	 * by fetching information from the `LookingGlassDevice`. Render this component using a `nap::OrthoCameraComponentInstance`.
	 *
	 * For more information on light field capture for the Looking Glass, please refer to:
	 * https://docs.lookingglassfactory.com/keyconcepts/capturing-a-lightfield/linear-light-field-capture
	 */
	class NAPAPI RenderLightFieldComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderLightFieldComponent, RenderLightFieldComponentInstance)
	public:
		ResourcePtr<RenderTexture2D>	mQuiltTexture = nullptr;					///< Property: 'QuiltTexture' the input quilt texture
		ResourcePtr<LookingGlassDevice>	mDevice = nullptr;							///< Property: 'LookingGlassDevice' the looking glass device
		bool							mOverscan = false;							///< Property: 'Overscan' enables overscan
		bool							mInvertQuilt = false;						///< Property: 'Overscan' inverts the quilt
		bool							mDebug = false;								///< Property: 'Debug' renders the unmodified quilt texture to the looking glass
	};


	/**
	 * RenderLightFieldComponentInstance
	 *
	 * Applies `nap::LightFieldShader` to the specified quilt `QuiltTexture`. The shader is fully calibrated
	 * by fetching information from the `LookingGlassDevice`. Render this component using a `nap::OrthoCameraComponentInstance`.
	 *
	 * For more information on light field capture for the Looking Glass, please refer to:
	 * https://docs.lookingglassfactory.com/keyconcepts/capturing-a-lightfield/linear-light-field-capture
	 */
	class NAPAPI RenderLightFieldComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderLightFieldComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderLightFieldComponentInstance based on the RenderLightFieldComponent resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderLightFieldComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Called by the Render Service. This component only supports `nap::OrthoCameraComponentInstance`.
		 * @return if the object can be rendered with the given camera
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

	protected:
		/**
		 * Draws the light field full screen to the currently active render target.
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		RenderService*				mRenderService = nullptr;			///< Render service
		RenderTexture2D*			mQuiltTexture = nullptr;			///< Reference to the input quilt texture
		
		MaterialInstanceResource	mMaterialInstanceResource;			///< Instance of the material, used to override uniforms for this instance
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.

		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		DummyMesh					mDummyMesh;							///< Dummy mesh for creating the post-processing shader pipeline
	};
}
