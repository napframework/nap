#include "cvcascadeclassifycomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::cascadeclassifycomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVCascadeClassifyComponent)
	RTTI_PROPERTY("CaptureComponent",	&nap::CVCascadeClassifyComponent::mCaptureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Adapter",			&nap::CVCascadeClassifyComponent::mAdapter,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MatrixIndex",		&nap::CVCascadeClassifyComponent::mMatrixIndex,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Path",				&nap::CVCascadeClassifyComponent::mPath,				nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

// nap::cascadeclassifycomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVCascadeClassifyComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVCascadeClassifyComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource and copy render texture
		CVCascadeClassifyComponent* resource = getComponent<CVCascadeClassifyComponent>();
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

		return true;
	}


	void CVCascadeClassifyComponentInstance::update(double deltaTime)
	{

	}


	void CVCascadeClassifyComponentInstance::onFrameCaptured(const CVFrameEvent& frameEvent)
	{
		const CVFrame* frame = frameEvent.findFrame(*mAdapter);
		if (frame == nullptr)
			return;
		const CVFrame& cv_frame = *frame;

		// Convert to grey-scale and equalize history
		cvtColor(cv_frame[0], mFrameGrey, cv::COLOR_BGR2GRAY);
		equalizeHist(mFrameGrey, mFrameGrey);

		std::vector<cv::Rect> faces;
		mClassifier.detectMultiScale(mFrameGrey, faces);
		nap::Logger::info("found %d: faces", faces.size());
	}
}