#pragma once

// Local Includes
#include "cvcaptureapi.h"
#include "cvevent.h"
#include "cvadapter.h"
#include "cvservice.h"
#include "cvcaptureerror.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>
#include <future>
#include <atomic>

namespace nap
{
	/**
	 * This device captures OpenCV frames in a background thread. 
	 * Add an adapter to the 'Adapters' property to capture a frame from that specific device. Multiple adapters can be specified.
	 * Every frame from every adapter is captured at the exact same point in time, allowing for perfect frame capture synchronization.
	 * 
	 * To receive frame events on the main thread use a nap::CVCaptureComponent and listen to the 'frameReceived' signal.
	 * Frames are automatically forwarded to the right component based on the nap::CVCaptureComponent 'Device' property.
	 * Listen to the 'frameCaptured' signal of this device to receive new frame events from the processing (background) thread directly.
	 */
	class NAPAPI CVCaptureDevice : public Device
	{
		RTTI_ENABLE(Device)
	public:

		/**
		 * Constructor
		 * @param service the service that receives all frame capture events.
		 */
		CVCaptureDevice(CVService& service);

		/**
		 * Initializes the capture device
		 * @param errorState contains the error if initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * Starts the capture device.
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override final;

		/**
		 * Stops the capture device.
		 */
		virtual void stop() override final;

		/**
		 * Checks if a new frame is available. 
		 * Note that grabbing frames manually might conflict with the automatic frame forwarding mechanism of this module.
		 * Frames can only be grabbed manually when there is no nap::CVCaptureComponent interested in frames from this specific device.
		 * @return if there is a new frame available.
		 */
		bool newFrame() const;

		/*
		 * Grabs the last captured frame. The result is copied over into the given target.
		 * This call performs a reference based copy operation by default that is thread safe.
		 * Set the 'copy' flag to true when a deep copy of the last frame is required.
		 * use newFrame() to figure out if a new frame is available before grabbing it.
		 * 
		 * Note that grabbing frames manually might conflict with the automatic frame forwarding mechanism of this module.
		 * Frames can only be grabbed manually when there is no nap::CVCaptureComponent interested in frames from this specific device.
		 * @param target updated to hold the last captured frame data if a new frame is available.
		 * @param copy if the latest frame contents are copied over into the given target.
         */
		void grab(CVFrameEvent& target, bool copy = false);

		/**
		 * Tells the capture thread to capture the next available frame.
		 * Typically you call capture after a successful frame grab operation.
		 * If 'AutoCapture' is turned on there is no need to call this function.
 		 */
		void capture();

		/**
		 * Sets an OpenCV capture property. This call is thread safe.
		 * The actual property is applied when a new frame is captured, not immediately.
		 * @param propID the property to set.
		 * @param value the new property value.
		 */
		void setProperty(CVAdapter& adapter, cv::VideoCaptureProperties propID, double value);

		/**
		 * @return the adapter at the given index as type T.
		 */
		template<typename T>
		const T& getAdapter(int index) const;

		/**
		 * @return the adapter at the given index as type T.
		 */
		template<typename T>
		T& getAdapter(int index);

		/**
		 * @return number of adapters
		 */
		int getAdapterCount() const											{ return static_cast<int>(mAdapters.size()); }

		/**
		 * Returns the last known capture task compute time in seconds.
		 * This includes the time it takes to complete the entire process cycle for all the adapters.
		 * @return capture task compute time.
		 */
		double getComputeTime() const;

		/**
		 * @return if any error is associated with this capture device.
		 */
		bool hasErrors() const												{ return mHasErrors; }

		/**
		 * Returns all errors currently associated with the given adapter.
		 * Only call this when hasErrors() returns true. 
		 * This call asserts when the adapter isn't part of this capture device as defined by the 'Adapters' property.
		 * @param adapter adapter to get errors for.
		 * @return all errors associated with the given adapter.
		 */
		CVCaptureErrorMap getErrors(const CVAdapter& adapter) const;

		/**
		 * Returns all errors currently associated with this capture device, thread safe.
		 * Only call this when hasErrors() returns true. 
		 * @return all errors associated with this capture device
		 */
		std::unordered_map<const CVAdapter*, CVCaptureErrorMap> getErrors() const;

		/**
		 * Occurs when a new frame is captured on the background thread.
		 * Listen to this signal when you want to process frame data on a background thread.
		 * Use the CVFrameEvent::copyTo() or CVFrameEvent::clone() method to duplicate 
		 * the actual content of a frame when received.
		 */
		nap::Signal<const CVFrameEvent&> frameCaptured;

		std::vector<nap::ResourcePtr<CVAdapter>> mAdapters;					///< Property: 'Adapters' all the video capture adapters.							
		bool					mAutoCapture = false;						///< Property: 'AutoCapture' if this device captures new frames automatically.

	private:
		CVFrameEvent			mCaptureMat;								///< The GPU / CPU matrix that holds the most recent captured video frame
		std::atomic<bool>		mCaptureFrame	= { true };					///< Proceed to next frame
		std::atomic<double>		mComputeTime	= { 0.0f };					///< Last known capture task compute time
		bool					mStopCapturing	= false;					///< Signals the capture thread to stop capturing video
		bool					mFrameAvailable = false;					///< If a new frame is captured

		std::future<void>		mCaptureTask;								///< The thread that monitor the read thread
		std::mutex				mCaptureMutex;								///< The mutex that safe guards the capture thread
		std::condition_variable	mCaptureCondition;							///< Used for telling the polling task to continue

		using PropertyMap = std::unordered_map<int, double>;			
		std::unordered_map<CVAdapter*, PropertyMap> mAdapterMap;			///< All properties to set for the given adapter

		std::atomic<bool>		mHasErrors = { false };						///< If any errors are associated with this device
		mutable std::mutex		mErrorMutex;								///< Mutex associated with setting / getting errors
		std::unordered_map<const CVAdapter*, CVCaptureErrorMap> mErrorMap;	///< All errors associated with a specific adapter

		CVService*				mService = nullptr;							///< OpenCV service

		/**
		 * Task that captures new frames
		 */
		void captureTask();

		/**
		 * Sets an error for the given adapter
		 * @param adapter adapter associated with the error
		 * @param error the error code
		 * @param msg error message
		 */
		void setError(const CVAdapter& adapter, CVCaptureError error, const std::string& msg);
	};

	using CVCaptureDeviceObjectCreator = rtti::ObjectCreator<CVCaptureDevice, CVService>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	const T& nap::CVCaptureDevice::getAdapter(int index) const
	{
		assert(index < mAdapters.size());
		T* adapter = rtti_cast<T>(mAdapters[index].get());
		assert(adapter != nullptr);
		return *adapter;
	}


	template<typename T>
	T& nap::CVCaptureDevice::getAdapter(int index)
	{
		assert(index < mAdapters.size());
		T* adapter = rtti_cast<T>(mAdapters[index].get());
		assert(adapter != nullptr);
		return *adapter;
	}
}
