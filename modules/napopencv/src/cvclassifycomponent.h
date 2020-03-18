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
	 *	cascadeclassifycomponent
	 */
	class NAPAPI CVClassifyComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVClassifyComponent, CVClassifyComponentInstance)
	public:

		nap::ComponentPtr<CVCaptureComponent> mCaptureComponent = nullptr;	///< Property: 'CaptureComponent' the component that receives the captured frames
		nap::ResourcePtr<CVAdapter> mAdapter = nullptr;						///< Property: 'Adapter' the adapter to render frame for
		int mMatrixIndex = 0;												///< Property: 'MatrixIndex' the OpenCV matrix of the adapter used for detection
		std::string mPath;													///< Property: 'Path' path to cascade classifier file
	};


	/**
	 * cascadeclassifycomponentInstance	
	 */
	class NAPAPI CVClassifyComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CVClassifyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~CVClassifyComponentInstance() override;

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

		/**
		 * @return classified (detected) objects
		 */
		std::vector<math::Rect> getObjects() const;

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
         * Task that captures new frames
		 */
		void detectTask();
	};
}
