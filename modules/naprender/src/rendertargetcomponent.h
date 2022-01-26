/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rendertexture2d.h"
#include "planemesh.h"
#include "rendertarget.h"
#include "materialinstance.h"
#include "renderablemesh.h"
#include "color.h"

// External Includes
#include <nap/resourceptr.h>
#include <entity.h>

// Local Includes
#include "renderwindow.h"

namespace nap
{
	// Forward Declares
	class RenderTargetComponentInstance;

	/**
	 * 
	 */
	class NAPAPI RenderTargetComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderTargetComponent, ComponentInstance)
	public:
		ResourcePtr<RenderWindow>		mReferenceWindow = nullptr;							///< Property: 'ReferenceWindow' reference window to sync up with
		ResourcePtr<RenderTexture2D>	mOutputTexture = nullptr;							///< Property: 'OutputTexture' the target of the render step
		MaterialInstanceResource		mMaterialInstanceResource;							///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		ERasterizationSamples			mRequestedSamples = ERasterizationSamples::One;		///< Property: 'Samples' The number of samples used during Rasterization. For better results enable 'SampleShading'
		RGBAColor8						mClearColor = { 255, 255, 255, 255 };				///< Property: 'ClearColor' the color that is used to clear the render target
	};


	/**
	 * Renders a scene to an output texture
	 */
	class NAPAPI RenderTargetComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderTargetComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderToTextureComponentInstance based on the RenderToTextureComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendertotexturecomponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the render target that is used to perform the render step	
		 */
		IRenderTarget& getTarget();

		/**
		 * @return the output texture
		 */
		RenderTexture2D& getOutputTexture(); 


	private:
		RenderService*				mRenderService = nullptr;			///< Render service
		nap::RenderTarget			mRenderTarget;						///< Internally managed render target
	};
}
