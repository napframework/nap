/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cvclassifycomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::cascadeclassifycomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVClassifyComponent)
	RTTI_PROPERTY("CaptureComponent",	&nap::CVClassifyComponent::mCaptureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Adapter",			&nap::CVClassifyComponent::mAdapter,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MatrixIndex",		&nap::CVClassifyComponent::mMatrixIndex,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Path",				&nap::CVClassifyComponent::mPath,				nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

// nap::cascadeclassifycomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVClassifyComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	CVClassifyComponentInstance::~CVClassifyComponentInstance()
	{
		{
			// Stop detection thread and notify worker
			std::lock_guard<std::mutex> lock(mClassifyMutex);
			mStopClassification = true;
		}
		mClassifyCondition.notify_one();

		// Wait till exit
		if (mClassifyTask.valid())
			mClassifyTask.wait();
	}


	bool CVClassifyComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource and copy render texture
		CVClassifyComponent* resource = getComponent<CVClassifyComponent>();
		mAdapter = resource->mAdapter.get();
		mMatrixIndex = resource->mMatrixIndex;

		// Ensure adapter is part of capture device
		if (!errorState.check(mCaptureComponent->getDevice().manages(*mAdapter), "%s: adapter: %s not part of %s",
			resource->mID.c_str(),
			mAdapter->mID.c_str(), mCaptureComponent->getDevice().mID.c_str()))
			return false;

		// Now ensure matrix capture is in range
		if (!errorState.check(mMatrixIndex < mAdapter->getMatrixCount(),
			"%s: matrix index out of range, adapter: %s has only %d matrices available", resource->mID.c_str(),
			mAdapter->mID.c_str(), mAdapter->getMatrixCount()))
			return false;

		// Load the cascade
		if (!errorState.check(mClassifier.load(resource->mPath), "%s: unable to load cascade: %s",
			this->mID.c_str(), resource->mPath.c_str()))
			return false;

		// Assign slot when new frame is captured
		mCaptureComponent->frameReceived.connect(mCaptureSlot);

		// Start capture
		mStopClassification = false;
		mClassify = false;
		mClassifyTask = std::async(std::launch::async, std::bind(&CVClassifyComponentInstance::detectTask, this));

		return true;
	}


	std::vector<math::Rect> CVClassifyComponentInstance::getObjects() const
	{
		std::lock_guard<std::mutex> lock(mObjectMutex);
		return mObjects;
	}


	void CVClassifyComponentInstance::onFrameCaptured(const CVFrameEvent& frameEvent)
	{
		const CVFrame* frame = frameEvent.findFrame(*mAdapter);
		if (frame == nullptr)
			return;

		// Store frame for processing
		{
			std::lock_guard<std::mutex> lock(mClassifyMutex);
			mCapturedFrame = *frame;
			mClassify = true;
		}
		mClassifyCondition.notify_one();
	}


	void CVClassifyComponentInstance::detectTask()
	{
		CVFrame process_frame, gray_frame;
		gray_frame.addNew();
		std::vector<cv::Rect> cv_objects;
		std::vector<math::Rect> na_objects;

		while (!mStopClassification)
		{
			// Wait for the detect condition to be true.
			// When this happens copy all the properties to set in order to release lock
			{
				std::unique_lock<std::mutex> lock(mClassifyMutex);
				mClassifyCondition.wait(lock, [this]()
				{
					return (mStopClassification || mClassify);
				});

				// Exit loop when exit has been triggered
				if (mStopClassification)
					break;

				// Copy and clear
				mCapturedFrame.copyTo(process_frame);
				mClassify = false;
			}

			// Convert to grey scale and equalize history
			cvtColor(process_frame[0], gray_frame[0], cv::COLOR_BGR2GRAY);
			equalizeHist(gray_frame[0], gray_frame[0]);

			// Detect and store
			mClassifier.detectMultiScale(gray_frame[0], cv_objects);

			// Copy over rects
			na_objects.clear();
			for (auto& rect : cv_objects)
				na_objects.emplace_back(math::Rect(rect.x, rect.y, rect.width, rect.height));

			std::lock_guard<std::mutex> lock(mObjectMutex);
			mObjects = na_objects;
		}
	}
}