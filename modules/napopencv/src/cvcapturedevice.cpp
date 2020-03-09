// Local Includes
#include "cvcapturedevice.h"

// External Includes
#include <nap/logger.h>
#include <nap/timer.h>

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVCaptureDevice)
	RTTI_CONSTRUCTOR(nap::CVService&)
	RTTI_PROPERTY("AutoCapture",				&nap::CVCaptureDevice::mAutoCapture,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowConnectionFailure",		&nap::CVCaptureDevice::mAllowConnectionFailure,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Adapters",					&nap::CVCaptureDevice::mAdapters,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void CVCaptureDevice::grab(CVFrameEvent& target, bool deepCopy)
	{
		// Copy last captured frame using a deep copy if requested.
		// The deep copy might be necessary because a weak copy allows
		// for the data to be updated by the capture loop whilst still processing on another thread.
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		mFrameAvailable = false;
		if (deepCopy)
		{
			mCaptureMat.copyTo(target);
			return;
		}
		target = mCaptureMat;
	}
	

	void CVCaptureDevice::capture()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mCaptureFrame = true;
		}
		mCaptureCondition.notify_one();
	}


	CVCaptureDevice::CVCaptureDevice(CVService& service)
	{
		mService = &service;
	}


	bool CVCaptureDevice::init(utility::ErrorState& errorState)
	{
		for (auto& adapter : mAdapters)
			adapter->mParent = this;
		return true;
	}


	bool CVCaptureDevice::start(utility::ErrorState& errorState)
	{
		// Register with service
		mService->registerCaptureDevice(*this);

		// Initialize property and error map
		mAdapterMap.reserve(mAdapters.size());
		mErrorMap.reserve(mAdapters.size());
		
		// Start and on success add adapter for capturing
		for (auto& adapter : mAdapters)
		{
			mErrorMap[adapter.get()] = {};
			if (!adapter->started())
			{
				std::string msg = utility::stringFormat("%s: Adapter: %s did not start", mID.c_str(), adapter->mID.c_str());
				if (!mAllowConnectionFailure)
				{
					errorState.fail(msg);
					return false;
				}

				// Set and log error
				setError(*adapter, CVCaptureError::OpenError, msg);
				nap::Logger::warn(msg);

				// Discard adapter as valid capture device
				continue;
			}

			// Create properties
			mAdapterMap[adapter.get()] = {};
		}

		// Start capture task
		mStopCapturing = false;
		mCaptureFrame = true;
		mCaptureTask = std::async(std::launch::async, std::bind(&CVCaptureDevice::captureTask, this));
		return true;
	}


	void CVCaptureDevice::setProperty(CVAdapter& adapter, cv::VideoCaptureProperties propID, double value)
	{
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		auto it = mAdapterMap.find(&adapter);
		if (it == mAdapterMap.end())
		{
			nap::Logger::warn("%s: unknown CVAdapter: %s", this->mID.c_str(), adapter.mID.c_str());
			return;
		}
		(*it).second[propID] = value;
	}


	double CVCaptureDevice::getComputeTime() const
	{
		return mComputeTime;
	}


	std::unordered_map<const CVAdapter*, nap::CVCaptureErrorMap> CVCaptureDevice::getErrors() const
	{
		std::lock_guard<std::mutex> lock(mErrorMutex);
		return mErrorMap;
	}


	nap::CVCaptureErrorMap CVCaptureDevice::getErrors(const CVAdapter& adapter) const
	{
		std::lock_guard<std::mutex> lock(mErrorMutex);
		auto it = mErrorMap.find(&adapter);
		assert(it != mErrorMap.end());
		return it->second;
	}


	void CVCaptureDevice::stop()
	{
		// Stop capturing thread and notify worker
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mStopCapturing = true;
		}
		mCaptureCondition.notify_one();

		// Wait till exit
		if (mCaptureTask.valid())
			mCaptureTask.wait();

		// Clear all properties
		mAdapterMap.clear();

		// Clear all errors
		mErrorMap.clear();
		mHasErrors = false;

		// Unregister capture device
		mService->removeCaptureDevice(*this);
	}


	bool CVCaptureDevice::newFrame() const
	{
		return mFrameAvailable;
	}


	void CVCaptureDevice::captureTask()
	{
		// Wait for playback to be enabled, a new frame request is issued or request to stop is made
		// Exit loop immediately when a stop is requested. Otherwise process next frame
		CVFrameEvent frame_event;
		frame_event.reserve(mAdapters.size());
		std::unordered_map<CVAdapter*, PropertyMap> properties;
		std::vector<nap::CVAdapter*> capture_adapters;
		bool set_properties = false;
		SystemTimer timer;

		while(!mStopCapturing)
		{
			{
				std::unique_lock<std::mutex> lock(mCaptureMutex);
				mCaptureCondition.wait(lock, [this]()
				{
					return (mStopCapturing || mCaptureFrame);
				});

				// Exit loop when exit has been triggered
				if (mStopCapturing)
					break;

				// Copy and clear properties
				properties = mAdapterMap;
				for (auto& prop : mAdapterMap)
					prop.second.clear();

				// Don't capture new frame immediately, only do so when 'AutoCapture' is turned on
				// and no decoding errors have been detected.
				mCaptureFrame = false;
			}

			// Apply properties for every adapter and grab next frame
			// Store adapters that are ready to be captured
			timer.reset();
			capture_adapters.clear();
			for (auto& adapter : properties)
			{
				nap::CVAdapter* cur_adapter = adapter.first;
				for (const auto& prop : adapter.second)
				{
					if (!cur_adapter->getCaptureDevice().set(prop.first, prop.second))
					{
						std::string msg = utility::stringFormat("%s unable to set property: %s to: %.02f", cur_adapter->mID.c_str(), prop.first, prop.second);
						setError(*cur_adapter, CVCaptureError::PropertyError, msg);
					}
				}
				
				if (!cur_adapter->getCaptureDevice().grab())
				{
					std::string msg = utility::stringFormat("%s: failed to grab frame from %s. Device disconnected or end of stream.", mID.c_str(), cur_adapter->mID.c_str());
					setError(*cur_adapter, CVCaptureError::GrabError, msg);
					continue;
				}
				capture_adapters.emplace_back(cur_adapter);
			}

			// Now retrieve frame (heaviest operation)
			nap::utility::ErrorState grab_error;
			frame_event.clear();
			auto it = capture_adapters.begin();
			while (it != capture_adapters.end())
			{
				// If frame capture operation fails, pop the frame and remove capture device
				frame_event.addFrame((*it)->retrieve(grab_error));
				if (frame_event.lastFrame().empty())
				{
					std::string msg = utility::stringFormat("%s: failed to decode frame", mID.c_str());
					setError(**it, CVCaptureError::DecodeError, msg);
					frame_event.popFrame();
					it = capture_adapters.erase(it);
					continue;
				}
				it++;
			}

			// No frames captured
			if (frame_event.empty())
				continue;

			// Notify listeners
			frameCaptured.trigger(frame_event);

			// Copy the frame, note that this performs a shallow copy of the frame instead of a deep copy.
			// It is therefore important that the capture device returns a clone of the last capture.
			{
				std::lock_guard<std::mutex> lock(mCaptureMutex);
				mCaptureMat = frame_event;
				mFrameAvailable = true;
			}

			// Notify adapters that a frame is copied.
			// Only do this for adapters for which the frame was retrieved successfully. 
			for (auto& adapter : capture_adapters)
				adapter->copied();

			// New frame is available
			mCaptureFrame	= mAutoCapture;
			mComputeTime	= timer.getElapsedTime();
		}
	}


	void CVCaptureDevice::setError(const CVAdapter& adapter, CVCaptureError error, const std::string& msg)
	{
		{
			std::lock_guard<std::mutex> lock(mErrorMutex);
			mErrorMap[&adapter][error] = msg;
		}
		mHasErrors = true;
	}
}