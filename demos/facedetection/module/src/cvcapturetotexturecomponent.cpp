#include "cvcapturetotexturecomponent.h"

// External Includes
#include <entity.h>
#include <cvframe.h>
#include <nap/logger.h>

// nap::cvdisplaycapturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVCaptureToTextureComponent)
	RTTI_PROPERTY("CaptureComponent",	&nap::CVCaptureToTextureComponent::mCaptureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderTexture",		&nap::CVCaptureToTextureComponent::mRenderTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Adapter",			&nap::CVCaptureToTextureComponent::mAdapter,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MatrixIndex",		&nap::CVCaptureToTextureComponent::mMatrixIndex,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::cvdisplaycapturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVCaptureToTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVCaptureToTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource and copy render texture
		CVCaptureToTextureComponent* resource = getComponent<CVCaptureToTextureComponent>();
		mRenderTexture  = resource->mRenderTexture.get();
		mAdapter		= resource->mAdapter.get();
		mMatrixIndex	= resource->mMatrixIndex;

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

		// Assign slot when new frame is captured
		mConversionFrame.addNew();
		mCaptureComponent->frameReceived.connect(mCaptureSlot);

		return true;
	}


	void CVCaptureToTextureComponentInstance::onFrameCaptured(const CVFrameEvent& frameEvent)
	{
		const CVFrame* frame = frameEvent.findFrame(*mAdapter);
		if (frame == nullptr)
			return;

		// Ensure channel count is the same
		const CVFrame& cv_frame = *frame;
		int cv_channels = cv_frame[mMatrixIndex].channels();
		int te_channels = mRenderTexture->getChannelCount();
		if (!(cv_channels == te_channels))
		{
			nap::Logger::warn("%s: invalid number of channels, got %d, expect: %d", mID.c_str(),
				cv_channels, te_channels);
			return;
		}

		// Ensure dimensions are the same
		glm::vec2 tex_size = mRenderTexture->getSize();
		if (cv_frame[mMatrixIndex].cols != tex_size.x || cv_frame[mMatrixIndex].rows != tex_size.y)
		{
			nap::Logger::warn("%s: invalid size, got %d:%d, expect: %d%d", mID.c_str(),
				tex_size.x,
				tex_size.y,
				cv_frame[mMatrixIndex].cols,
				cv_frame[mMatrixIndex].rows);
			return;
		}

		// Convert to RGB and flip vertically
		cv::cvtColor(cv_frame[mMatrixIndex], mConversionFrame[0], cv::COLOR_BGR2RGB);
		cv::flip(mConversionFrame[0], mConversionFrame[0], 0);
		
		// Update texture
		mRenderTexture->update(mConversionFrame[0].getMat(cv::ACCESS_READ).data);
	}
}