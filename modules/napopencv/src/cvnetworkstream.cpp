/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cvnetworkstream.h"
#include "cvcapturedevice.h"

#include <nap/logger.h>
#include <mathutils.h>

// nap::cvvideoadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVNetworkStream)
	RTTI_PROPERTY("Resize",			&nap::CVNetworkStream::mResize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::CVNetworkStream::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Link",			&nap::CVNetworkStream::mLink,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVNetworkStream::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	int CVNetworkStream::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVNetworkStream::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	bool CVNetworkStream::reconnect(utility::ErrorState& error)
	{
		return restart(error);
	}


	bool CVNetworkStream::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mLink, api), "%s: Unable to open network stream: %s", mID.c_str(), mLink.c_str()))
			return false;

		// Reset some internally managed variables
		mCaptureFrame	= CVFrame(1, this);
		mOutputFrame	= CVFrame(1, this);

		return true;
	}


	CVFrame CVNetworkStream::onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error)
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
}