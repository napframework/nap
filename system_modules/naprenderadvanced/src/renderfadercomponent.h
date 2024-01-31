/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rendercomponent.h>
#include <renderablemesh.h>
#include <materialinstance.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <transformcomponent.h>
#include <perspcameracomponent.h>
#include <rect.h>
#include <componentptr.h>
#include <nomesh.h>

namespace nap
{
	class RenderFaderComponentInstance;
	
	/**
	 * Resource part of RenderFaderComponent
	 * 
	 * Renders a alpha-blended full-screen triangle in front of all previously rendered objects. Uses a `nap::NoMesh`
	 * internally to avoid using buffers. The `FadedToBlackSignal` and `FadedOutSignal` members can be used to
	 * hook up any user callbacks in response to changes of transition states.
	 *
	 * Ensure this component is excluded from shadow rendering passes using render tags, and is rendered last (on top
	 * of previously rendered objects) using render layers. When setup this way, the component can be rendered without
	 * a depth buffer: `EDepthMode::NoReadWrite`.
	 */
	class NAPAPI RenderFaderComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
			DECLARE_COMPONENT(RenderFaderComponent, RenderFaderComponentInstance)
	public:
		float								mFadeDuration = 2.0f;				///< Property: 'Fade' Duration of the fade transition in seconds.
		ComponentPtr<PerspCameraComponent>	mCamera;							///< Property: 'Camera' Reference camera.
		MaterialInstanceResource			mMaterialInstanceResource;			///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		bool								mStartBlack = true;					///< Property: 'StartBlack' Whether to fade out of black on startup
	};


	/**
	 * Instance part of RenderFaderComponent
	 * 
	 * Renders a alpha-blended full-screen triangle in front of all previously rendered objects. Uses a `nap::NoMesh`
	 * internally to avoid using buffers. The `FadedToBlackSignal` and `FadedOutSignal` members can be used to
	 * hook up any user callbacks in response to changes of transition states.
	 *
	 * Ensure this component is excluded from shadow rendering passes using render tags, and is rendered last (on top
	 * of previously rendered objects) using render layers. When setup this way, the component can be rendered without
	 * a depth buffer: `EDepthMode::NoReadWrite`.
	 */
	class NAPAPI RenderFaderComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)

	public:
		RenderFaderComponentInstance(EntityInstance& entity, Component& component);

		/**
		 * Initializes this component.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the local transform of this component such that it represents the location of the back plane. 
		 * This ensures the depth sorter considers this object to be behind other components inside the box.
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Called by the Render Service. This component only supports `nap::QuiltCameraComponentInstance`.
		 * @return if the object can be rendered with the given camera
		 */
		virtual bool isSupported(CameraComponentInstance& camera) const override;

		/**
		 * @return whether a fade-in/out transition is currently happening.
		 */
		bool isFadeActive() const;

		/**
		 * Starts a fade-in transition.
		 */
		void startFade();

		ComponentInstancePtr<PerspCameraComponent> mCamera = { this, &RenderFaderComponent::mCamera };

		nap::Signal<> mFadedToBlackSignal;						///< Triggered when a fade transition has just finished fading to black.
		nap::Signal<> mFadedOutSignal;							///< Triggered when a fade transition has just finished fading out.

	protected:
		/**
		 * @return current material used when drawing the mesh.
		 */
		MaterialInstance& getMaterialInstance()					{ return mRenderableMesh.getMaterialInstance(); }

		/**
		 * Creates a renderable mesh that can be used to switch to another mesh and/or material at runtime. This function should be called from
		 * init() functions on other components, and the result should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param materialInstance The material instance that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		/**
		 * Creates a renderable mesh that can be used to switch to another mesh at runtime. The material remains the same material as the one that
		 * was already set on the RenderFaderComponent.. This function should be called from init() functions on other components, and the result
		 * should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState);

		/**
		 * Switches the mesh and/or the material that is rendered. The renderable mesh should be created through createRenderableMesh, and must
		 * be created from an init() function.
		 * @param mesh The mesh that was retrieved through createRenderableMesh.
		 */
		void setMesh(const RenderableMesh& mesh);

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
	 	 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		MaterialInstance						mMaterialInstance;				///< The MaterialInstance as created from the resource. 
		RenderableMesh							mRenderableMesh;				///< The currently active renderable mesh, either set during init() or set by setMesh.
		std::unique_ptr<NoMesh>					mNoMesh;

		RenderFaderComponent*					mResource = nullptr;
		RenderService*							mRenderService = nullptr;

		float									mElapsedTime = 0.0f;
		float									mFadeStartTime = 0.0f;

		// Internal fade transition state enum
		enum EFadeState
		{
			Off = 0,
			In = 1,
			Out = 2
		};
		EFadeState								mFadeState = EFadeState::Off;	///< The current fade transition state
	};
}
