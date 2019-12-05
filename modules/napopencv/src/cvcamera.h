#pragma once

// Local Includes
#include "cvvideocapture.h"

// External Includes
#include <thread>
#include <future>
#include <atomic>

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
		 * Grabs the last captured frame if new. The result is stored in the given target.
		 * Internally the frame is removed from the queue. 
		 * If there is no new capture the target is not updated and the function returns false.
		 * This operation hands over ownership of the captured frame.
		 * This call is thread safe and can be called every frame in to check for a new frame.
		 * @param target updated to hold the last captured frame data if a new frame is available.
		 * @return if the target is updated with the contents of a new captured frame.
		 */
		bool grab(cv::UMat& target);

		/**
		 * Tells the capture thread to capture the next available frame.
		 * This is a non-blocking call!
		 */
		void captureNextFrame();

		/**
		 * Sets and applies new camera settings the next time a frame is captured.
		 * @param settings the camera settings to set and apply
		 * @param error contains the error message if the operation fails
		 * @return if the operation succeeded
		 */
		void setSettings(const nap::CVCameraSettings& settings);

		/**
		 * Returns the active, currently in use camera settings.
		 * @return the current camera settings.
		 */
		void getSettings(nap::CVCameraSettings& settings);

		bool				mConvertRGB = true;			///< Property: 'ConvertRGB' if the frame is converted into RGB
		bool				mFlipHorizontal = false;	///< Property: 'FlipHorizontal' flips the frame on the x-axis
		bool				mFlipVertical = false;		///< Property: 'FlipVertical' flips the frame on the y-axis
		bool				mApplySettings = false;		///< Property: 'ApplySettings' If the camera settings are applied on startup
		nap::uint			mDeviceIndex = 0;			///< Property: 'DeviceIndex' Capture device index
		nap::uint			mFrameWidth = 640;			///< Property: 'FrameWidth' width of the frame in pixels
		nap::uint			mFrameHeight = 480;			///< Property: 'FrameHeight' height of the frame in pixels
		CVCameraSettings	mCameraSettings;			///< Property: 'Settings' all configurable camera settings
		bool				mShowDialog = false;		///< Property: 'ShowDialog' if the external camera settings dialog is shown on startup

	protected:
		/**
		 * Called by the capture device when the camera needs to be opened.
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;
		
		/**
		 * Called by the capture device on start, when the camera is open.
		 * Starts processing threads.
		 */
		virtual bool onStart(cv::VideoCapture& captureDevice, nap::utility::ErrorState& error) override;

		/**
		 * Called before closing the device, stop processing threads.
		 */
		virtual void onStop() override;

	private:
		cv::UMat				mCaptureMat;					///< The GPU / CPU matrix that holds the most recent captured video frame
		bool					mCaptureFrame = true;			///< Proceed to next frame
		std::atomic<bool>		mFrameAvailable = { false };	///< If a new frame is captured
		std::atomic<bool>		mSettingsDirty  = { false };	///< If settings need to be updated

		std::future<void>		mCaptureTask;					///< The thread that monitor the read thread
		std::mutex				mCaptureMutex;					///< The mutex that safe guards the capture thread
		std::mutex				mSettingsMutex;					///< Guards getting / setting of camera settings
		bool					mStopCapturing = false;			///< Signals the capture thread to stop capturing video
		std::condition_variable	mCaptureCondition;				///< Used for telling the polling task to continue

		/**
		 * Captures new frames.
		 */
		void capture();

		/**
		 * Tries to apply current camera settings.
		 * @param camera settings to push to hardware
		 * @param error contains the error if operation fails.
		 * @return if the operation succeeded
		 */
		bool applySettings(const CVCameraSettings& settings, utility::ErrorState& error);

		/**
		 * Synchronizes the camera settings.
		 * This call ensures the given camera settings reflect the current state of the hardware.
		 * The result of this operation greatly depends on the underlying API, OS and hardware itself.
		 */
		void syncSettings(CVCameraSettings&  outSettings);
	};
}
