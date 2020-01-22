#include "cvcapturetotexturecomponent.h"

// External Includes
#include <entity.h>
#include <cvframe.h>
#include <nap/logger.h>

// nap::cvdisplaycapturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVCaptureToTextureComponent)
	RTTI_PROPERTY("CaptureComponent",	&nap::CVCaptureToTextureComponent::mCaptureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderTexture",		&nap::CVCaptureToTextureComponent::mRenderTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AdapterIndex",		&nap::CVCaptureToTextureComponent::mAdapterIndex,		nap::rtti::EPropertyMetaData::Default)
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
		mAdapterIndex	= resource->mAdapterIndex;
		mMatrixIndex	= resource->mMatrixIndex;

		// Ensure adapter index is in range
		int adapter_count = mCaptureComponent->getDevice().getAdapterCount();
		if (!errorState.check(mAdapterIndex < adapter_count || adapter_count == 0, "adapter index out of range"))
			return false;

		// Now ensure matrix capture is in range
		nap::CVAdapter& cap_adap = mCaptureComponent->getDevice().getAdapter<nap::CVAdapter>(mAdapterIndex);
		if (!errorState.check(mMatrixIndex < cap_adap.getMatrixCount(),
			"matrix index out of range, adapter: %s has only %d matrices available", cap_adap.mID.c_str(), cap_adap.getMatrixCount()))
			return false;

		// Assign slot when new frame is captured
		mCaptureComponent->frameReceived.connect(mCaptureSlot);

		return true;
	}


	void CVCaptureToTextureComponentInstance::onFrameCaptured(const CVFrameEvent& frame)
	{
		// Ensure indices are within bounds
		assert(mAdapterIndex < frame.getCount());
		assert(mMatrixIndex < frame[mAdapterIndex].getCount());

		// Extract frame
		const CVFrame& cv_frame = frame.getFrame(mAdapterIndex);		

		// Ensure channel count is the same
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

		// Ensure depth is the same
		int cv_depth = cv_frame[mMatrixIndex].depth();
		if (mRenderTexture->mFormat == RenderTexture2D::EFormat::Depth)
		{
			if (cv_depth != CV_32F)
			{
				nap::Logger::warn("%s: invalid bit depth, expected: 32 bit float", mID.c_str());
				return;
			}
		}
		else
		{
			if (cv_depth != CV_8U)
			{
				nap::Logger::warn("%s: invalid bit depth, expected: 8 bit unsigned", mID.c_str());
				return;
			}
		}
		
		// Update texture
		cv::Mat cpu_mat = cv_frame[mMatrixIndex].getMat(cv::ACCESS_READ);
		mRenderTexture->update(cpu_mat.data);
	}
}