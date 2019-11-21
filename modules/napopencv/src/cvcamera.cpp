#include "cvcamera.h"

#include <utility/stringutils.h>
#include <nap/logger.h>

RTTI_BEGIN_STRUCT(nap::CVCameraParameters)
	RTTI_PROPERTY("AutoExposure",		&nap::CVCameraParameters::mAutoExposure,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness",			&nap::CVCameraParameters::mBrightness,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Contrast",			&nap::CVCameraParameters::mContrast,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Saturation",			&nap::CVCameraParameters::mSaturation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Gain",				&nap::CVCameraParameters::mGain,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Exposure",			&nap::CVCameraParameters::mExposure,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS(nap::CVCamera)
	RTTI_PROPERTY("ConvertRGB",			&nap::CVCamera::mConvertRGB,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",		&nap::CVCamera::mFlipHorizontal,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",		&nap::CVCamera::mFlipVertical,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplyParameters",	&nap::CVCamera::mApplyParameters,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DeviceIndex",		&nap::CVCamera::mDeviceIndex,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Parameters",			&nap::CVCamera::mCameraParameters,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameWidth",			&nap::CVCamera::mFrameWidth,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameHeight",		&nap::CVCamera::mFrameHeight,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	std::string nap::CVCameraParameters::toString() const
	{
		rtti::TypeInfo type = RTTI_OF(nap::CVCameraParameters);
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


	bool CVCamera::grab(cv::UMat& target)
	{
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		if (!mNewFrame)
			return false;

		target = mCaptureMat;
		mNewFrame = false;
		return true;
	}


	bool CVCamera::applyParameters(utility::ErrorState& error)
	{
		bool return_v = true;
		if (!setProperty(cv::CAP_PROP_BRIGHTNESS, (double)mCameraParameters.mBrightness))
		{
			return_v = false;
			error.fail("unable to set brightness");
		}

		if (!setProperty(cv::CAP_PROP_CONTRAST, (double)mCameraParameters.mContrast))
		{
			return_v = false;
			error.fail("unable to set contrast");
		}

		if (!setProperty(cv::CAP_PROP_SATURATION, (double)mCameraParameters.mSaturation))
		{
			return_v = false;
			error.fail("unable to set saturation");
		}
		
		if (!setProperty(cv::CAP_PROP_GAIN, (double)mCameraParameters.mGain))
		{
			return_v = false;
			error.fail("unable to set gain");
		}

		if (!setProperty(cv::CAP_PROP_EXPOSURE, (double)mCameraParameters.mExposure))
		{
			return_v = false;
			error.fail("unable to set exposure");
		}

		if (!setProperty(cv::CAP_PROP_AUTO_EXPOSURE, (double)mCameraParameters.mAutoExposure))
		{
			return_v = false;
			error.fail("unable to set auto-exposure");
		}

		return return_v;
	}


	bool CVCamera::setParameters(const nap::CVCameraParameters& parameters, utility::ErrorState& error)
	{
		mCameraParameters = parameters;
		return applyParameters(error);
	}


	void CVCamera::syncParameters()
	{
		mCameraParameters.mAutoExposure = static_cast<bool>(getProperty(cv::CAP_PROP_AUTO_EXPOSURE));
		mCameraParameters.mBrightness	= getProperty(cv::CAP_PROP_BRIGHTNESS);
		mCameraParameters.mContrast		= getProperty(cv::CAP_PROP_CONTRAST);
		mCameraParameters.mExposure		= getProperty(cv::CAP_PROP_EXPOSURE);
		mCameraParameters.mGain			= getProperty(cv::CAP_PROP_GAIN);
		mCameraParameters.mSaturation	= getProperty(cv::CAP_PROP_SATURATION);
	}


	bool CVCamera::showSettings()
	{
		return setProperty(cv::CAP_PROP_SETTINGS, 0.0);
	}


	bool CVCamera::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(static_cast<int>(mDeviceIndex), api),
			"unable to open video capture device: %d", mDeviceIndex))
			return false;
		return true;
	}


	bool CVCamera::onStart(cv::VideoCapture& captureDevice, nap::utility::ErrorState& error)
	{
		// Set capture dimensions
		if (!getCaptureDevice().set(cv::CAP_PROP_FRAME_WIDTH, (double)mFrameWidth))
		{
			error.fail("unable to set video capture frame width to: %d", mFrameWidth);
			return false;
		}

		if (!getCaptureDevice().set(cv::CAP_PROP_FRAME_HEIGHT, (double)mFrameHeight))
		{
			error.fail("unable to set video capture frame height to: %d", mFrameHeight);
			return false;
		}

		// Apply parameters if requested
		utility::ErrorState paramError;
		if (mApplyParameters && !applyParameters(paramError))
			nap::Logger::warn("%s: %s", mID.c_str(), paramError.toString().c_str());

		// Update parameters based on current config and print
		syncParameters();
		rtti::TypeInfo type_info = RTTI_OF(nap::CVCameraParameters);
		nap::Logger::info("%s: %s", mID.c_str(), mCameraParameters.toString().c_str());

		mStopCapturing = false;
		mCaptureTask = std::async(std::launch::async, std::bind(&CVCamera::capture, this));

		return true;
	}


	void CVCamera::onStop()
	{
		// Stop capturing thread
		mStopCapturing = true;
		if (mCaptureTask.valid())
			mCaptureTask.wait();
	}


	void CVCamera::capture()
	{
		cv::UMat cap_frame;
		while (!mStopCapturing)
		{
			// Fetch frame
			getCaptureDevice() >> cap_frame;
			if(cap_frame.empty())
				continue;

			// Flip vertically
			if(mFlipVertical)
				cv::flip(cap_frame, cap_frame, 0);

			// Flip Horizontally
			if (mFlipHorizontal)
				cv::flip(cap_frame, cap_frame, 1);
			
			// Convert to RGB
			if(mConvertRGB)
				cv::cvtColor(cap_frame, cap_frame, cv::COLOR_BGR2RGB);
			
			// Deep-Copy the captured frame into our shared captured material
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			cap_frame.copyTo(mCaptureMat);
			mNewFrame = true;
		}
	}
}
