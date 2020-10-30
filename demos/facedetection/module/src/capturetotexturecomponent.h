/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <cvcapturecomponent.h>
#include <cvadapter.h>
#include <componentptr.h>
#include <rendertexture2d.h>
#include <rendertotexturecomponent.h>
#include <cvclassifycomponent.h>
#include <uniforminstance.h>

namespace nap
{
	// Forward Declares
	class NAPAPI CaptureToTextureComponentInstance;

	/**
	 * Resource part of the CaptureToTextureComponent.
	 *
	 * Ensures that the material used to render the captured frame (together with the detected blobs) 
	 * contains the most recent frame and classification data.
	 */
	class CaptureToTextureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CaptureToTextureComponent, CaptureToTextureComponentInstance)
	public:

		nap::ComponentPtr<CVCaptureComponent> mCaptureComponent = nullptr;			///< Property: 'CaptureComponent' the component that receives the captured frames
		nap::ComponentPtr<RenderToTextureComponent> mRenderComponent = nullptr;		///< Property: 'RenderComponent' the component that renders the detected blobs
		nap::ComponentPtr<CVClassifyComponent> mClassifyComponent = nullptr;		///< Property: 'ClassifyComponent' the component that performs object detection (classification)
		nap::ResourcePtr<CVAdapter> mAdapter = nullptr;								///< Property: 'Adapter' the adapter to render frame for
		nap::ResourcePtr<RenderTexture2D> mRenderTexture = nullptr;					///< Property: 'RenderTexture' the texture to render the captured frames to
		int mMatrixIndex = 0;														///< Property: 'MatrixIndex' the OpenCV matrix index of the adapter to render
	};


	/**
	 * Instance part of the CaptureToTextureComponent.
	 *
	 * Ensures that the material used to render the captured frame (together with the detected blobs)
	 * contains the most recent frame and classification data. 
	 *
	 * The frame data is updated when the 'mCaptureSlot' is called by the CVCaptureComponent.
	 * Detected blobs are fetched, mapped and pushed to the material on update.
	 */
	class NAPAPI CaptureToTextureComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CaptureToTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize this instance based on the resource
		 * @param errorState holds the error message when initialization fails
		 * @return if the instance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the material uniform inputs based on the detected number of blobs.
		 * @param deltaTime time in seconds in between calls.
		 */
		virtual void update(double deltaTime) override;

		// Component that receives captured frames
		nap::ComponentInstancePtr<CVCaptureComponent> mCaptureComponent			= { this, &CaptureToTextureComponent::mCaptureComponent };

		// Component that applies object detection
		nap::ComponentInstancePtr<CVClassifyComponent> mClassifyComponent		= { this, &CaptureToTextureComponent::mClassifyComponent };

		// Component that renders received frame + detected blobs
		nap::ComponentInstancePtr<RenderToTextureComponent> mRenderComponent	= { this, &CaptureToTextureComponent::mRenderComponent };

		// Frame captures are uploaded to this texture
		nap::RenderTexture2D* mRenderTexture = nullptr;

	private:
		// Slot that is called when a new frame is captured (main thread)
		nap::Slot<const CVFrameEvent&> mCaptureSlot = { this, &CaptureToTextureComponentInstance::onFrameCaptured };
		void onFrameCaptured(const CVFrameEvent& frameEvent);
		
		// The adapter we should receive a frame from
		nap::CVAdapter* mAdapter = nullptr;						///< Pointer to the OpenCV capture device
		int mMatrixIndex = 0;									///< OpenCV sample matrix, defaults to 0
		UniformIntInstance* mBlobCountUniform = nullptr;		///< OpenCV blob count uniform
		UniformStructArrayInstance* mBlobsUniform = nullptr;	///< Blobs uniform struct array
		CVFrame mConversionFrame;
	};
}
