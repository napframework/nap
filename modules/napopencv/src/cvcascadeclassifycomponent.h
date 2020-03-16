#pragma once

#include <component.h>
#include <cvadapter.h>
#include <componentptr.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect.hpp>
#include <cvcapturecomponent.h>

namespace nap
{
	class CVCascadeClassifyComponentInstance;

	/**
	 *	cascadeclassifycomponent
	 */
	class NAPAPI CVCascadeClassifyComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVCascadeClassifyComponent, CVCascadeClassifyComponentInstance)
	public:

		nap::ComponentPtr<CVCaptureComponent> mCaptureComponent = nullptr;	///< Property: 'CaptureComponent' the component that receives the captured frames
		nap::ResourcePtr<CVAdapter> mAdapter = nullptr;						///< Property: 'Adapter' the adapter to render frame for
		int mMatrixIndex = 0;												///< Property: 'MatrixIndex' the OpenCV matrix index of the adapter to render
		std::string mPath;													///< Property: 'Path' path to cascade classifier file
	};


	/**
	 * cascadeclassifycomponentInstance	
	 */
	class NAPAPI CVCascadeClassifyComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CVCascadeClassifyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~CVCascadeClassifyComponentInstance() override;

		/**
		 * Initialize cascadeclassifycomponentInstance based on the cascadeclassifycomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the cascadeclassifycomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update cascadeclassifycomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		nap::ComponentInstancePtr<CVCaptureComponent> mCaptureComponent = { this, &CVCascadeClassifyComponent::mCaptureComponent };

	private:
		void onFrameCaptured(const CVFrameEvent& frameEvent);
		nap::Slot<const CVFrameEvent&> mCaptureSlot =					{ this, &CVCascadeClassifyComponentInstance::onFrameCaptured };
		cv::CascadeClassifier mClassifier;								///< OpenCV cascade classifier
		nap::CVAdapter* mAdapter = nullptr;								///< OpenCV capture adapter
		int mMatrixIndex = 0;											///< OpenCV matrix index
		cv::UMat mFrameGrey;											///< OpenCV grey frame

		std::future<void> mDetectTask;									///< The task that performs classification
		std::mutex mDetectMutex;										///< The mutex that safe guards the capture thread
		std::condition_variable	mDetectCondition;						///< Used for telling the polling task to continue
		bool mStopDetection = false;									///< If the detection should stop
		bool mDetect = false;											///< Proceed to next frame
		CVFrame mCapturedFrame;											///< Latest frame that is captured

		/**
         * Task that captures new frames
		 */
		void detectTask();
	};
}
