#include "cvcamera.h"

#include <utility/stringutils.h>
#include <nap/logger.h>

RTTI_BEGIN_STRUCT(nap::CVCameraSettings)
	RTTI_PROPERTY("AutoExposure",		&nap::CVCameraSettings::mAutoExposure,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness",			&nap::CVCameraSettings::mBrightness,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Contrast",			&nap::CVCameraSettings::mContrast,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Saturation",			&nap::CVCameraSettings::mSaturation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Gain",				&nap::CVCameraSettings::mGain,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Exposure",			&nap::CVCameraSettings::mExposure,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS(nap::CVCamera)
	RTTI_PROPERTY("ShowDialog",			&nap::CVCamera::mShowDialog,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplySettings",		&nap::CVCamera::mApplySettings,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplyDimensions",	&nap::CVCamera::mApplyDimensions,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DeviceIndex",		&nap::CVCamera::mDeviceIndex,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Settings",			&nap::CVCamera::mCameraSettings,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameWidth",			&nap::CVCamera::mFrameWidth,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameHeight",		&nap::CVCamera::mFrameHeight,				nap::rtti::EPropertyMetaData::Default)
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


	CVCamera::~CVCamera()			{ }


	void CVCamera::setSettings(const nap::CVCameraSettings& settings)
	{
		setProperty(cv::CAP_PROP_BRIGHTNESS,	(double)settings.mBrightness);
		setProperty(cv::CAP_PROP_CONTRAST,		(double)settings.mContrast);
		setProperty(cv::CAP_PROP_SATURATION,	(double)settings.mSaturation);
		setProperty(cv::CAP_PROP_GAIN,			(double)settings.mGain);
		setProperty(cv::CAP_PROP_EXPOSURE,		(double)settings.mExposure);
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


	bool CVCamera::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(static_cast<int>(mDeviceIndex), api),
			"unable to open video capture device: %d", mDeviceIndex))
			return false;

		// Set capture dimensions
		if (mApplyDimensions)
		{
			if (!captureDevice.set(cv::CAP_PROP_FRAME_WIDTH, (double)mFrameWidth))
			{
				error.fail("unable to set video capture frame width to: %d", mFrameWidth);
				return false;
			}

			if (!captureDevice.set(cv::CAP_PROP_FRAME_HEIGHT, (double)mFrameHeight))
			{
				error.fail("unable to set video capture frame height to: %d", mFrameHeight);
				return false;
			}
		}

		if (mShowDialog && !captureDevice.set(cv::CAP_PROP_SETTINGS, 0.0))
			nap::Logger::warn("unable to show camera settings dialog");

		// Apply camera settings if requested
		utility::ErrorState paramError;
		if (mApplySettings)
		{
			setSettings(mCameraSettings);
		}
		return true;
	}


	bool CVCamera::onRetrieve(cv::VideoCapture& captureDevice, cv::UMat& outFrame, utility::ErrorState& error)
	{
		if (!captureDevice.retrieve(outFrame))
		{
			error.fail("%s: no new frame available", mID.c_str());
			return false;
		}
		return true;
	}
}
