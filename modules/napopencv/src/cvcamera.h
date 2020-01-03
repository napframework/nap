#pragma once

// Local Includes
#include "cvvideocapture.h"

// External Includes
#include <thread>
#include <future>
#include <atomic>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Configurable OpenCV camera settings.
	 */
	struct NAPAPI CVCameraSettings
	{
		bool	mAutoExposure	= true;		///< Property: 'AutoExposure' if auto exposure is turned on or off
		float	mBrightness		= 1.0f;		///< Property: 'Brightness' camera brightness
		float	mContrast		= 1.0f;		///< Property: 'Contrast' camera contrast
		float	mSaturation		= 1.0f;		///< Property: 'Saturation' camera saturation
		float	mGain			= 1.0f;		///< Property: 'Gain' camera gain
		float	mExposure		= 1.0f;		///< Property: 'Exposure' camera exposure

		/**
		 * @return the settings as a human readable string.
		 */
		std::string toString() const;
	};


	/**
	 * Captures a video stream from a web cam or other peripheral video capture device. 
	 * The captured video frame is stored on the GPU when hardware acceleration is available (OpenCL).
	 * Otherwise the captured video frame is stored on the CPU.
	 * This device captures the video stream on a background thread, call grab() to grab the last recorded video frame.
	 * Camera settings can be provided on startup by enabling 'ApplySettings'.
	 * After startup the camera settings reflect the current state of the hardware.
	 */
	class NAPAPI CVCamera : public CVVideoCapture
	{
		RTTI_ENABLE(CVVideoCapture)
	public:

		// Stops the device
		virtual ~CVCamera() override;

		/**
		 * Sets and applies new camera settings the next time a frame is captured.
		 * @param settings the camera settings to set and apply
		 * @param error contains the error message if the operation fails
		 * @return if the operation succeeded
		 */
		void setSettings(const nap::CVCameraSettings& settings);

		/**
		 * Returns the active, currently in use camera settings.
		 * Note that depending on your hardware this call can be slow.
		 * @return the current camera settings.
		 */
		void getSettings(nap::CVCameraSettings& settings);

		bool				mApplySettings = false;		///< Property: 'ApplySettings' if the camera settings are applied on startup
		bool				mDefaultResolution = true;	///< Property: 'DefaultResolution' if the default camera resolution is used, when set to false the specified 'Resolution' is enforced.
		nap::uint			mDeviceIndex = 0;			///< Property: 'DeviceIndex' capture device index
		glm::ivec2			mResolution = { 640, 480 };	///< Property: 'Resolution' camera record resolution, only used when 'DefaultResolution' is turned off
		CVCameraSettings	mCameraSettings;			///< Property: 'Settings' all configurable camera settings
		bool				mShowDialog = false;		///< Property: 'ShowDialog' if the external camera settings dialog is shown on startup

	protected:
		virtual int getMatrixCount() override			{ return 1; }

		/**
		 * Called by the capture device when the camera needs to be opened.
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;

		/**
		 * This method decodes and returns the just grabbed frame.
		 * Needs to be implemented in a derived class.
		 * Return false when decoding fails, otherwise return true.
		 * The 'outFrame' should contain the decoded frame on success.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual bool onRetrieve(cv::VideoCapture& captureDevice, CVFrame& outFrame, utility::ErrorState& error) override;

	private:
		std::atomic<bool>		mSettingsDirty			= { false };	///< If settings need to be updated
		bool					mLocalSettings;
	};
}
