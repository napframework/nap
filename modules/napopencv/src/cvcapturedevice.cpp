// Local Includes
#include "cvcapturedevice.h"

// External Includes
#include <nap/logger.h>

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVCaptureDevice)
	RTTI_CONSTRUCTOR(nap::CVService&)
	RTTI_PROPERTY("AutoCapture",	&nap::CVCaptureDevice::mAutoCapture,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowFailure",	&nap::CVCaptureDevice::mAllowFailure,	nap::rtti::EPropertyMetaData::Default)	
	RTTI_PROPERTY("Adapters",		&nap::CVCaptureDevice::mAdapters,		nap::rtti::EPropertyMetaData::Required)
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

		// Initialize property map
		mCaptureMap.clear();
		mCaptureMap.reserve(mAdapters.size());

		// Initialize error map
		mErrorMap.clear();
		mErrorMap.reserve(mAdapters.size());
		
		// Open every adapter
		bool ready = false;
		for (auto& adapter : mAdapters)
		{
			mErrorMap[adapter.get()]   = {};
			mCaptureMap[adapter.get()] = {};

			if (!(adapter->open(errorState)))
			{
				// Set and log error
				setError(*adapter, CVCaptureError::OpenError, errorState.toString());
				if (mAllowFailure)
					continue;
				return false;
			}
			ready = true;
		}

		// Start capture task if at least one adapter opened successfully
		if(ready)
			startCapture();
		return true;
	}


	void CVCaptureDevice::stop()
	{
		// Stop capture task
		stopCapture();

		// Close all open adapters and reset properties
		for (auto& it : mCaptureMap)
		{
			it.first->close();
			it.second = {};
		}

		// Unregister capture device
		mService->removeCaptureDevice(*this);
	}


	void CVCaptureDevice::setProperty(CVAdapter& adapter, cv::VideoCaptureProperties propID, double value)
	{
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		auto it = mCaptureMap.find(&adapter);
		assert(it != mCaptureMap.end());
		(*it).second[propID] = value;
	}


	bool CVCaptureDevice::restart(nap::CVAdapter& adapter, utility::ErrorState& error)
	{
		// Stop capture task
		assert(manages(adapter));
		stopCapture();

		// Try to open and when opened, reset errors and state
		// On failure to open, erase as capture candidate
		adapter.close();
		if (!adapter.open(error))
		{
			setError(adapter, CVCaptureError::OpenError, error.toString());
			startCapture();
			return false;
		}

		// Clear errors
		mErrorMap[&adapter] = {};
		startCapture();
		return true;
	}


	bool CVCaptureDevice::hasErrors(const CVAdapter& adapter) const
	{
		assert(manages(adapter));
		if (!hasErrors())
			return false;

		std::lock_guard<std::mutex> lock(mErrorMutex);
		auto it = mErrorMap.find(&adapter);
		assert(it != mErrorMap.end());
		return !(it->second.empty());
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


	bool CVCaptureDevice::newFrame() const
	{
		return mFrameAvailable;
	}


	void CVCaptureDevice::startCapture()
	{
		// Start capture
		mStopCapturing	= false;
		mCaptureFrame	= true;
		mCaptureTask	= std::async(std::launch::async, std::bind(&CVCaptureDevice::captureTask, this));
	}


	void CVCaptureDevice::stopCapture()
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
	}


	void CVCaptureDevice::captureTask()
	{
		// Wait for playback to be enabled, a new frame request is issued or request to stop is made
		// Exit loop immediately when a stop is requested. Otherwise process next frame
		CVFrameEvent frame_event;
		frame_event.reserve(mAdapters.size());

		// Copy of the capture map
		std::unordered_map<CVAdapter*, PropertyMap> properties;

		// All valid capture adapters (without errors and open)
		std::vector<nap::CVAdapter*> capture_adapters;

		// First wait for a capture request, this is set to true  after calling capture() or when 'AutoCapture' is turned on.
		// All properties that need to be applied are copied over in a safe block before continue.
		// After that properties are applied and a frame is grabbed. If grabbing fails, the device is closed and
		// an error is reported. Only adapters that managed to grab a frame are considered valid for a subsequent capture.
		// If no frame is captured at all the process is paused, until a new capture() resquest is issued.
		while(!mStopCapturing)
		{
			// Wait for the capture condition to be true.
			// When this happens copy all the properties to set in order to release lock
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
				properties = mCaptureMap;
				for (auto& prop : mCaptureMap)
					prop.second.clear();

				// Don't capture new frame immediately, only do so when 'AutoCapture' is turned on
				// or when capture() is called externally.
				mCaptureFrame = mAutoCapture;
			}

			// Ensure at least one device is open for capturing
			bool capture_ready = false;
			for (auto& adapter : properties)
			{
				nap::CVAdapter* cur_adapter = adapter.first;
				capture_ready = cur_adapter->isOpen();
			}
			
			// If no device is open at this point there's no point in running this loop
			if (!capture_ready)
				break;

			// Apply properties for every adapter and grab next frame
			// Store adapters that are ready to be captured
			capture_adapters.clear();
			for (auto& adapter : properties)
			{	
				// Get adapter and ensure it's open
				nap::CVAdapter* cur_adapter = adapter.first;
				if (!(cur_adapter->isOpen()))
					continue;

				// Apply all properties
				for (const auto& prop : adapter.second)
				{
					if (!cur_adapter->getCaptureDevice().set(prop.first, prop.second))
					{
						std::string msg = utility::stringFormat("%s: Unable to set property: %d to: %.02f", cur_adapter->mID.c_str(), prop.first, prop.second);
						setError(*cur_adapter, CVCaptureError::PropertyError, msg);
					}
				}
				
				// Attempt to grab frame, close adapter is operation fails.
				// Closing the adapter ensures it is skipped on the next capture operation
				if (!cur_adapter->getCaptureDevice().grab())
				{
					if (cur_adapter->mCloseOnCaptureError)
						cur_adapter->close();

					std::string msg = utility::stringFormat("%s: Device disconnected or end of stream.", cur_adapter->mID.c_str());
					setError(*cur_adapter, CVCaptureError::GrabError, msg);
					continue;
				}

				// Valid capture adapter
				capture_adapters.emplace_back(cur_adapter);
			}

			// Now retrieve frame (heaviest operation)
			nap::utility::ErrorState capture_error;
			frame_event.clear();
			auto it = capture_adapters.begin();
			while (it != capture_adapters.end())
			{
				// If frame capture operation fails, pop the frame and remove capture device
				frame_event.addFrame((*it)->retrieve(capture_error));
				if (frame_event.lastFrame().empty())
				{
					setError(**it, CVCaptureError::DecodeError, capture_error.toString());
					frame_event.popFrame();
					it = capture_adapters.erase(it);
					continue;
				}
				it++;
			}

			// New frame captured from one of the adapters
			if (!frame_event.empty())
			{
				// Notify listeners
				frameCaptured.trigger(frame_event);

				// Copy the frame, note that this performs a shallow copy of the frame instead of a deep copy.
				// It is therefore important that the capture device returns a clone of the last capture.
				std::lock_guard<std::mutex> lock(mCaptureMutex);
				mCaptureMat = frame_event;
				mFrameAvailable = true;
			}

			// Notify adapters that a frame is copied.
			// Only do this for adapters for which the frame was retrieved successfully. 
			for (auto& adapter : capture_adapters)
				adapter->copied();
		}
	}


	bool CVCaptureDevice::manages(const nap::CVAdapter& adapter) const
	{
		// Check if adapter is managed by this capture device
		auto found_adapter = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto& it)
		{
			return it == &adapter;
		});
		return found_adapter != mAdapters.end();
	}


	void CVCaptureDevice::setError(CVAdapter& adapter, CVCaptureError error, const std::string& msg)
	{
		{
			std::lock_guard<std::mutex> lock(mErrorMutex);
			mErrorMap[&adapter][error] = getCurrentDateTime().toString() + msg;
		}
		mHasErrors = true;
	}
}