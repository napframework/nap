// Local Includes
#include "cvcameraadapter.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_STRUCT(nap::CVCameraSettings)
	RTTI_PROPERTY("AutoExposure",		&nap::CVCameraSettings::mAutoExposure,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness",			&nap::CVCameraSettings::mBrightness,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Contrast",			&nap::CVCameraSettings::mContrast,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Saturation",			&nap::CVCameraSettings::mSaturation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Gain",				&nap::CVCameraSettings::mGain,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Exposure",			&nap::CVCameraSettings::mExposure,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::cvcameraadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVCameraAdapter)
	RTTI_PROPERTY("ShowDialog",			&nap::CVCameraAdapter::mShowDialog,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ConvertRGB",			&nap::CVCameraAdapter::mConvertRGB,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",		&nap::CVCameraAdapter::mFlipHorizontal,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",		&nap::CVCameraAdapter::mFlipVertical,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DeviceIndex",		&nap::CVCameraAdapter::mDeviceIndex,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DefaultResolution",	&nap::CVCameraAdapter::mDefaultResolution,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resolution",			&nap::CVCameraAdapter::mResolution,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resize",				&nap::CVCameraAdapter::mResize,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",				&nap::CVCameraAdapter::mSize,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplySettings",		&nap::CVCameraAdapter::mApplySettings,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Settings",			&nap::CVCameraAdapter::mCameraSettings,		nap::rtti::EPropertyMetaData::Default)
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


	bool CVCameraAdapter::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	int CVCameraAdapter::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVCameraAdapter::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	void CVCameraAdapter::setSettings(const nap::CVCameraSettings& settings)
	{
		setProperty(cv::CAP_PROP_BRIGHTNESS, (double)settings.mBrightness);
		setProperty(cv::CAP_PROP_CONTRAST, (double)settings.mContrast);
		setProperty(cv::CAP_PROP_SATURATION, (double)settings.mSaturation);
		setProperty(cv::CAP_PROP_GAIN, (double)settings.mGain);
		setProperty(cv::CAP_PROP_EXPOSURE, (double)settings.mExposure);
		setProperty(cv::CAP_PROP_AUTO_EXPOSURE, (double)settings.mAutoExposure);
	}


	void CVCameraAdapter::getSettings(nap::CVCameraSettings& settings)
	{
		settings.mAutoExposure	= getProperty(cv::CAP_PROP_AUTO_EXPOSURE) > 0;
		settings.mBrightness	= getProperty(cv::CAP_PROP_BRIGHTNESS);
		settings.mContrast		= getProperty(cv::CAP_PROP_CONTRAST);
		settings.mExposure		= getProperty(cv::CAP_PROP_EXPOSURE);
		settings.mGain			= getProperty(cv::CAP_PROP_GAIN);
		settings.mSaturation	= getProperty(cv::CAP_PROP_SATURATION);
	}


	bool CVCameraAdapter::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(static_cast<int>(mDeviceIndex), api),
			"unable to open video capture device: %d", mDeviceIndex))
			return false;

		// Set capture dimensions
		if (!mDefaultResolution)
		{
			if (!captureDevice.set(cv::CAP_PROP_FRAME_WIDTH, (double)(mResolution.x)))
			{
				error.fail("unable to set video capture frame width to: %d", mResolution.x);
				return false;
			}

			if (!captureDevice.set(cv::CAP_PROP_FRAME_HEIGHT, (double)(mResolution.y)))
			{
				error.fail("unable to set video capture frame height to: %d", mResolution.y);
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


	bool CVCameraAdapter::onRetrieve(cv::VideoCapture& captureDevice, CVFrame& outFrame, utility::ErrorState& error)
	{
		assert(outFrame.getCount() >= getMatrixCount());
		if (!captureDevice.retrieve(mFrame[0]))
		{
			error.fail("%s: no new frame available", mID.c_str());
			return false;
		}

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize)
		{
			cv::resize(mFrame[0], outFrame[0], cv::Size(mSize.x, mSize.y));
		}
		else
		{
			outFrame[0] = mFrame[0];
		}

		// Convert to RGB
		if (mConvertRGB)
			cv::cvtColor(outFrame[0], outFrame[0], cv::COLOR_BGR2RGB);

		// Flip horizontal
		if (mFlipHorizontal)
			cv::flip(outFrame[0], outFrame[0], 1);

		// Flip vertical
		if (mFlipVertical)
			cv::flip(outFrame[0], outFrame[0], 0);

		return true;
	}
}