/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cvvideo.h"
#include "cvcapturedevice.h"

#include <nap/logger.h>
#include <mathutils.h>

// nap::cvvideoadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideo)
	RTTI_PROPERTY("Resize",			&nap::CVVideo::mResize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::CVVideo::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("File",			&nap::CVVideo::mFile,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVVideo::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	bool CVVideo::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mFile, api), "%s: Unable to open video file: %s", mID.c_str(), mFile.c_str()))
			return false;

		// Reset some internally managed variables
		mCurrentFrame	= 0;
		mCaptureFrame	= CVFrame(1, this);
		mOutputFrame	= CVFrame(1, this);

		return true;
	}


	bool CVVideo::changeVideo(const std::string& video, nap::utility::ErrorState& error)
	{
		mFile = video;
		return restart(error);
	}


	int CVVideo::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVVideo::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	float CVVideo::getFramerate() const
	{
		return static_cast<float>(getProperty(cv::CAP_PROP_FPS));
	}


	float CVVideo::getLength()
	{
		return static_cast<float>(geFrameCount()) / getFramerate();
	}


	int CVVideo::geFrameCount() const
	{
		return static_cast<int>(getProperty(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT));
	}


	void CVVideo::reset()
	{
		setFrame(0);
	}


	void CVVideo::setFrame(int frame)
	{
		// Clamp to range and set as property
		int req_frame = nap::math::clamp<int>(frame, 0, geFrameCount() - 1);
		setProperty(cv::CAP_PROP_POS_FRAMES, static_cast<double>(req_frame));
		getParent().capture();
	}


	int CVVideo::getFrame()
	{
		return mCurrentFrame;
	}


	void CVVideo::setTime(float time)
	{
		setFrame(static_cast<int>(time * getFramerate()));
	}


	float CVVideo::getTime()
	{
		return static_cast<float>(mCurrentFrame) / getFramerate();
	}


	CVFrame CVVideo::onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error)
	{
		// Retrieve currently stored frame
		if (!captureDevice.retrieve(mCaptureFrame[0]))
		{
			error.fail("%s: No new frame available", mID.c_str());
			return CVFrame();
		}

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize && (mCaptureFrame[0].cols != mSize.x || mCaptureFrame[0].rows != mSize.y))
			cv::resize(mCaptureFrame[0], mOutputFrame[0], cv::Size(mSize.x, mSize.y));
		else
			mOutputFrame[0] = mCaptureFrame[0];

		return mOutputFrame.clone();
	}


	void CVVideo::onCopy()
	{
		mCurrentFrame = getProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES) - 1;
	}


	void CVVideo::onClose()
	{
		mCurrentFrame = 0;
	}

}