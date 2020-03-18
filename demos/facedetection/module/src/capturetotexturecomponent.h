#pragma once

// External Includes
#include <cvcapturecomponent.h>
#include <cvadapter.h>
#include <componentptr.h>
#include <rendertexture2d.h>
#include <rendertotexturecomponent.h>
#include <cvclassifycomponent.h>

namespace nap
{
	// Forward Declares
	class NAPAPI CaptureToTextureComponentInstance;

	/**
	 *	cvdisplaycapturecomponent
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
	 * cvdisplaycapturecomponentInstance	
	 */
	class NAPAPI CaptureToTextureComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CaptureToTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize this instance based on the resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the instance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Component that receives frame captures
		nap::ComponentInstancePtr<CVCaptureComponent> mCaptureComponent			= { this, &CaptureToTextureComponent::mCaptureComponent };

		// Component that applies object detection
		nap::ComponentInstancePtr<CVClassifyComponent> mClassifyComponent		= { this, &CaptureToTextureComponent::mClassifyComponent };

		// Component that renders received frame + detected blobs
		nap::ComponentInstancePtr<RenderToTextureComponent> mRenderComponent	= { this, &CaptureToTextureComponent::mRenderComponent };

		// Texture the received capture is uploaded to
		nap::RenderTexture2D* mRenderTexture = nullptr;

	private:
		// Slot that is called when a new frame is captured (main thread)
		nap::Slot<const CVFrameEvent&> mCaptureSlot = { this, &CaptureToTextureComponentInstance::onFrameCaptured };
		void onFrameCaptured(const CVFrameEvent& frameEvent);
		
		// The adapter we should receive a frame from
		nap::CVAdapter* mAdapter = nullptr;
		int mMatrixIndex = 0;
		CVFrame mConversionFrame;
	};
}
