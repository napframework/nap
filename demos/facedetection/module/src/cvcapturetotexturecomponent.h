#pragma once

// External Includes
#include <cvcapturecomponent.h>
#include <componentptr.h>
#include <rendertexture2d.h>
#include <opencv2/core/mat.hpp>

namespace nap
{
	class NAPAPI CVCaptureToTextureComponentInstance;

	/**
	 *	cvdisplaycapturecomponent
	 */
	class CVCaptureToTextureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVCaptureToTextureComponent, CVCaptureToTextureComponentInstance)
	public:

		nap::ComponentPtr<CVCaptureComponent> mCaptureComponent;		///< Property: 'CaptureComponent' the component that receives the captured frames
		nap::ResourcePtr<RenderTexture2D> mRenderTexture;				///< Property: 'RenderTexture' the texture to render the captured frames to
		int mAdapterIndex  = 0;											///< Property: 'AdapterIndex' the index of the adapter to render the frame from
		int mMatrixIndex = 0;											///< Property: 'MatrixIndex' the opencv matrix index of the adapter to render
	};


	/**
	 * cvdisplaycapturecomponentInstance	
	 */
	class NAPAPI CVCaptureToTextureComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CVCaptureToTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize cvdisplaycapturecomponentInstance based on the cvdisplaycapturecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the cvdisplaycapturecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		nap::ComponentInstancePtr<CVCaptureComponent> mCaptureComponent = { this, &CVCaptureToTextureComponent::mCaptureComponent };
		nap::RenderTexture2D* mRenderTexture = nullptr;

	private:
		nap::Slot<const CVFrameEvent&> mCaptureSlot = { this, &CVCaptureToTextureComponentInstance::onFrameCaptured };
		void onFrameCaptured(const CVFrameEvent& newFrame);
		int mAdapterIndex = 0;
		int mMatrixIndex = 0;
	};
}
