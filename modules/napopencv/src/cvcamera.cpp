/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "cvcamera.h"
#include "cvcapturedevice.h"

// External Includes
#include <nap/logger.h>
#include <nap/timer.h>
#include <opencv2/video.hpp>

RTTI_BEGIN_STRUCT(nap::CVCameraSettings)
	RTTI_PROPERTY("AutoExposure",		&nap::CVCameraSettings::mAutoExposure,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness",			&nap::CVCameraSettings::mBrightness,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Contrast",			&nap::CVCameraSettings::mContrast,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Saturation",			&nap::CVCameraSettings::mSaturation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Gain",				&nap::CVCameraSettings::mGain,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Exposure",			&nap::CVCameraSettings::mExposure,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::cvcameraadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVCamera)
	RTTI_PROPERTY("ShowDialog",			&nap::CVCamera::mShowDialog,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DeviceIndex",		&nap::CVCamera::mDeviceIndex,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Codec",				&nap::CVCamera::mCodec,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OverrideResolution",	&nap::CVCamera::mOverrideResolution,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resolution",			&nap::CVCamera::mResolution,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resize",				&nap::CVCamera::mResize,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",				&nap::CVCamera::mSize,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplySettings",		&nap::CVCamera::mApplySettings,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Settings",			&nap::CVCamera::mCameraSettings,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	std::string nap::CVCameraSettings::toString() const
	{
		rtti::TypeInfo type = RTTI_OF(nap::CVCameraSettings);
		std::string return_v;
		for (const rtti::Property& property : type.get_properties())
		{
			rtti::Variant value = property.get_value(*this);
			return_v += utility::stringFormat("%s: %s ", property.get_name().to_string().c_str(),
				value.to_string().c_str());
		}
		return return_v;
	}


	bool CVCamera::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	int CVCamera::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVCamera::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	void CVCamera::setSettings(const nap::CVCameraSettings& settings)
	{
		setProperty(cv::CAP_PROP_BRIGHTNESS, (double)settings.mBrightness);
		setProperty(cv::CAP_PROP_CONTRAST, (double)settings.mContrast);
		setProperty(cv::CAP_PROP_SATURATION, (double)settings.mSaturation);
		setProperty(cv::CAP_PROP_GAIN, (double)settings.mGain);
		setProperty(cv::CAP_PROP_EXPOSURE, (double)settings.mExposure);
		setProperty(cv::CAP_PROP_AUTO_EXPOSURE, (double)settings.mAutoExposure);
	}


	void CVCamera::getSettings(nap::CVCameraSettings& settings)
	{
		settings.mAutoExposure	= getProperty(cv::CAP_PROP_AUTO_EXPOSURE) > 0;
		settings.mBrightness	= getProperty(cv::CAP_PROP_BRIGHTNESS);
		settings.mContrast		= getProperty(cv::CAP_PROP_CONTRAST);
		settings.mExposure		= getProperty(cv::CAP_PROP_EXPOSURE);
		settings.mGain			= getProperty(cv::CAP_PROP_GAIN);
		settings.mSaturation	= getProperty(cv::CAP_PROP_SATURATION);
	}


	bool CVCamera::reconnect(utility::ErrorState& error)
	{
		return restart(error);
	}


	float CVCamera::getFPS() const
	{
		return mFramerate;
	}


	bool CVCamera::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		// Open capture device
		if (!error.check(captureDevice.open(static_cast<int>(mDeviceIndex), api),
			"%s: Unable to open video capture device: %d", mID.c_str(), mDeviceIndex))
			return false;

		// Set codec
		if (!mCodec.empty())
		{
			if (mCodec.length() != 4)
			{
				error.fail("Invalid codec, requires exactly 4 characters, for example: 'MJPG'");
				return false;
			}

			int codec_id = cv::VideoWriter::fourcc(mCodec[0], mCodec[1], mCodec[2], mCodec[3]);
			if (!captureDevice.set(cv::CAP_PROP_FOURCC, codec_id))
			{
				error.fail("Unsupported codec: %s", mCodec.c_str());
				return false;
			}
		}

		// Set capture dimensions
		if (mOverrideResolution)
		{
			if (!captureDevice.set(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(mResolution.x)))
			{
				error.fail("Unable to set video capture frame width to: %d", mResolution.x);
				return false;
			}

			if (!captureDevice.set(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(mResolution.y)))
			{
				error.fail("Unable to set video capture frame height to: %d", mResolution.y);
				return false;
			}
		}

		if (mShowDialog && !captureDevice.set(cv::CAP_PROP_SETTINGS, 0.0))
			nap::Logger::warn("Unable to show camera settings dialog");

		// Apply camera settings if requested
		utility::ErrorState paramError;
		if (mApplySettings)
		{
			setSettings(mCameraSettings);
		}

		// Reset frame states
		mCaptureFrame = CVFrame(1, this);
		mOutputFrame  = CVFrame(1, this);

		// Reset fps counter
		mTicksum = 0.0;
		mTickIdx = 0;
		mTicks = std::array<double, 20>();

		return true;
	}


	CVFrame CVCamera::onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error)
	{
		// Try to retrieve new frame
		if (!captureDevice.retrieve(mCaptureFrame[0]))
		{
			error.fail("%s: No new frame available", mID.c_str());
			return CVFrame();
		}

		// Get capture delta_time
		double delta_time = mTimer.getElapsedTime();
		mTimer.reset();

		// Update framerate
		mTicksum -= mTicks[mTickIdx];			// subtract value falling off
		mTicksum += delta_time;					// add new value
		mTicks[mTickIdx] = delta_time;			// save new value so it can be subtracted later */
		if (++mTickIdx == mTicks.size())		// inc buffer index
			mTickIdx = 0;

		// Store framerate
		mFramerate = static_cast<double>(mTicks.size()) / mTicksum;

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize && (mCaptureFrame[0].cols != mSize.x || mCaptureFrame[0].rows != mSize.y))
			cv::resize(mCaptureFrame[0], mOutputFrame[0], cv::Size(mSize.x, mSize.y));
		else
			mOutputFrame[0] = mCaptureFrame[0];

		// Clone frame
		return mOutputFrame.clone();
	}
}