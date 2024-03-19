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
#include <rect.h>
#include <componentptr.h>
#include <emptymesh.h>
#include <nap/timer.h>

namespace nap
{
	class RenderFaderComponentInstance;
	
	/**
	 * Resource part of RenderFaderComponent
	 * 
	 * Renders a alpha-blended full-screen triangle in front of all previously rendered objects. Uses a `nap::EmptyMesh`
	 * internally to avoid using buffers. The `FadedIn` and `FadedOut` signals can be used to
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
		float								mFadeDuration = 2.0f;				///< Property: 'FadeDuration' Duration of the fade transition in seconds.
		RGBColorFloat						mFadeColor = { 0.0f, 0.0f, 0.0f };	///< Property: 'FadeColor' Fade color
		bool								mFadeIn = true;						///< Property: 'FadeIn' Whether to fade in from 'FadeColor' on startup
	};


	/**
	 * Instance part of RenderFaderComponent
	 * 
	 * Renders a alpha-blended full-screen triangle in front of all previously rendered objects. Uses a `nap::EmptyMesh`
	 * internally to avoid using buffers. The `FadedIn` and `FadedOut` signals can be used to
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
		// Fade state
		enum class EFadeState: nap::uint8
		{
			FadingIn	= 0,		///< Currently fading in
			FadedIn		= 1,		///< Completely faded in (no effect)
			FadingOut	= 2,		///< Currently fading out
			FadedOut	= 3			///< Completely faded out
		};

		// Constructor
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
		 * @return current fade state
		 */
		EFadeState getState() const					{ return mFadeState; }

		/**
		 * Starts a fade-in transition.
		 */
		void fadeIn()								{ mFadeState = EFadeState::FadingIn; mTimer.reset(); }

		/**
		 * Starts a fade-out transition.
		 */
		void fadeOut()								{ mFadeState = EFadeState::FadingOut; mTimer.reset(); }

		nap::Signal<> mFadedIn;						///< Triggered when a fade transition finished fading from 'FadeColor'
		nap::Signal<> mFadedOut;					///< Triggered when a fade transition finished fading to 'FadeColor'

	protected:
		/**
		 * Renders the triangle mesh using the fade material.
	 	 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		MaterialInstance						mMaterialInstance;				///< The MaterialInstance as created from the resource. 
		RenderableMesh							mRenderableMesh;				///< The currently active renderable mesh, either set during init() or set by setMesh.
		EmptyMesh								mEmptyMesh;						///< The mesh to render
		MaterialInstanceResource				mMaterialInstanceResource;		///< Resource used to initialize the material instance

		RenderFaderComponent*					mResource = nullptr;
		RenderService*							mRenderService = nullptr;

		nap::SteadyTimer						mTimer;
		UniformFloatInstance*					mAlphaUniform = nullptr;

		EFadeState mFadeState = EFadeState::FadedIn;	///< The current fade transition state
	};
}
