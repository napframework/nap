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
	RTTI_PROPERTY("ConvertRGB",			&nap::CVCamera::mConvertRGB,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",		&nap::CVCamera::mFlipHorizontal,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",		&nap::CVCamera::mFlipVertical,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ApplySettings",		&nap::CVCamera::mApplySettings,				nap::rtti::EPropertyMetaData::Default)
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


	bool CVCamera::setSettings(const nap::CVCameraSettings& settings, utility::ErrorState& error)
	{
		mCameraSettings = settings;
		return applySettings(error);
	}


	void CVCamera::syncSettings()
	{
		mCameraSettings.mAutoExposure	= static_cast<bool>(getProperty(cv::CAP_PROP_AUTO_EXPOSURE));
		mCameraSettings.mBrightness		= getProperty(cv::CAP_PROP_BRIGHTNESS);
		mCameraSettings.mContrast		= getProperty(cv::CAP_PROP_CONTRAST);
		mCameraSettings.mExposure		= getProperty(cv::CAP_PROP_EXPOSURE);
		mCameraSettings.mGain			= getProperty(cv::CAP_PROP_GAIN);
		mCameraSettings.mSaturation		= getProperty(cv::CAP_PROP_SATURATION);
	}


	bool CVCamera::showSettingsDialog()
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

		// Apply settings if requested
		utility::ErrorState paramError;
		if (mApplySettings && !applySettings(paramError))
			nap::Logger::warn("%s: %s", mID.c_str(), paramError.toString().c_str());

		// Update settings based on current config and print to console
		syncSettings();
		rtti::TypeInfo type_info = RTTI_OF(nap::CVCameraSettings);
		nap::Logger::info("%s: %s", mID.c_str(), mCameraSettings.toString().c_str());

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


	bool CVCamera::grab(cv::UMat& target)
	{
		// Check if a new frame is available
		if (!mFrameAvailable)
			return false;

		// Copy last captured frame using a deep copy.
		// Again, the deep copy is necessary because a weak copy allows
		// for the data to be updated by the capture loop whilst still processing on another thread.
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		mCaptureMat.copyTo(target);
		mFrameAvailable = false;
		return true;
	}


	void CVCamera::capture()
	{
		// Start the processing loop until stop capturing is called 
		// or capturing new frames fails (when the camera faults or is disconnected).
		// Create the capture frame before starting the loop to ensure no storage on the GPU or CPU
		// is allocated every time we capture a frame. 
		cv::UMat cap_frame;
		while (!mStopCapturing)
		{
			// Fetch frame
			getCaptureDevice() >> cap_frame;
			if (cap_frame.empty())
			{
				nap::Logger::error("failed to capture frame, aborting: %s", mID.c_str());
				break;
			}

			// Flip vertically
			if(mFlipVertical)
				cv::flip(cap_frame, cap_frame, 0);

			// Flip Horizontally
			if (mFlipHorizontal)
				cv::flip(cap_frame, cap_frame, 1);
			
			// Convert to RGB
			if(mConvertRGB)
				cv::cvtColor(cap_frame, cap_frame, cv::COLOR_BGR2RGB);
			
			// Deep copy the captured frame to our storage matrix.
			// This updates the data of our storage container and ensures the same dimensionality.
			// We need to perform a deep-copy because if we choose to use a shallow copy, 
			// by the time the frame is grabbed the data 'mCaptureMat' points to could have changed, 
			// as it references the same data as in 'cap_frame'. And the 'cap_frame' process loop already started.
			// Performing a deep_copy ensures that when the data is grabbed it will contain the latest full processed frame.
			//
			// Alternatively, we could make the 'cap_frame' variable local to this loop, but that creates
			// more overhead than the copy below.
			{
				std::lock_guard<std::mutex> lock(mCaptureMutex);
				cap_frame.copyTo(mCaptureMat);
			}
			mFrameAvailable = true;
		}
	}


	bool CVCamera::applySettings(utility::ErrorState& error)
	{
		bool return_v = true;
		if (!setProperty(cv::CAP_PROP_BRIGHTNESS, (double)mCameraSettings.mBrightness))
		{
			return_v = false;
			error.fail("unable to set brightness");
		}

		if (!setProperty(cv::CAP_PROP_CONTRAST, (double)mCameraSettings.mContrast))
		{
			return_v = false;
			error.fail("unable to set contrast");
		}

		if (!setProperty(cv::CAP_PROP_SATURATION, (double)mCameraSettings.mSaturation))
		{
			return_v = false;
			error.fail("unable to set saturation");
		}

		if (!setProperty(cv::CAP_PROP_GAIN, (double)mCameraSettings.mGain))
		{
			return_v = false;
			error.fail("unable to set gain");
		}

		if (!setProperty(cv::CAP_PROP_EXPOSURE, (double)mCameraSettings.mExposure))
		{
			return_v = false;
			error.fail("unable to set exposure");
		}

		if (!setProperty(cv::CAP_PROP_AUTO_EXPOSURE, (double)mCameraSettings.mAutoExposure))
		{
			return_v = false;
			error.fail("unable to set auto-exposure");
		}
		return return_v;
	}
}
