#pragma once

#include <component.h>
#include <cvadapter.h>
#include <componentptr.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect.hpp>
#include <cvcapturecomponent.h>
#include <rect.h>

namespace nap
{
	class CVClassifyComponentInstance;

	/**
	 * OpenCV Cascade Classifier, can be used to detect objects in a frame using a HaarCascade profile. 
	 * The 'Path' property is a file-link that should point to a valid HaarCascade profile on disk.
	 */
	class NAPAPI CVClassifyComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVClassifyComponent, CVClassifyComponentInstance)
	public:

		nap::ComponentPtr<CVCaptureComponent> mCaptureComponent = nullptr;	///< Property: 'CaptureComponent' the component that receives the captured frames
		nap::ResourcePtr<CVAdapter> mAdapter = nullptr;						///< Property: 'Adapter' the adapter to run the classification algorithm on
		int mMatrixIndex = 0;												///< Property: 'MatrixIndex' the OpenCV matrix index, defaults to 0
		std::string mPath;													///< Property: 'Path' path to cascade classifier file
	};


	/**
	 * Detects objects in a frame using a HaarCascade profile.
	 * Classification is performed on a background thread when the 'CaptureComponent' receives a new frame. 
	 * Call 'getObjects' to get a list of classified (detected) objects.
	 */
	class NAPAPI CVClassifyComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CVClassifyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~CVClassifyComponentInstance() override;

		/**
		 * Initializes the classification component.
		 * @param errorState contains the error when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns a list of classified (detected) objects. 
		 * This call is thread safe and can be called every frame.
		 * @return list of classified (detected) objects.
		 */
		std::vector<math::Rect> getObjects() const;

		// Resolved link to the OpenCV capture component.
		nap::ComponentInstancePtr<CVCaptureComponent> mCaptureComponent = { this, &CVClassifyComponent::mCaptureComponent };

	private:
		void onFrameCaptured(const CVFrameEvent& frameEvent);
		nap::Slot<const CVFrameEvent&> mCaptureSlot =					{ this, &CVClassifyComponentInstance::onFrameCaptured };
		
		cv::CascadeClassifier mClassifier;								///< OpenCV cascade classifier
		nap::CVAdapter* mAdapter = nullptr;								///< OpenCV capture adapter
		int mMatrixIndex = 0;											///< OpenCV matrix index

		std::future<void> mClassifyTask;								///< The task that performs classification
		std::mutex mClassifyMutex;										///< The mutex that safe guards the capture thread
		std::condition_variable	mClassifyCondition;						///< Used for telling the polling task to continue
		bool mStopClassification = false;								///< If the detection should stop
		bool mClassify = false;											///< Proceed to next frame
		CVFrame mCapturedFrame;											///< Latest frame that is captured
		std::vector<math::Rect> mObjects;								///< All detected objects
		mutable std::mutex mObjectMutex;								///< The mutex that safe guards the capture thread

		/**
         * Classification task runs in the background.
		 */
		void detectTask();
	};
}
