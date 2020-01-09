// Local Includes
#include "cvvideocapture.h"

// External Includes
#include <nap/logger.h>
#include <nap/timer.h>

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideoCapture)
	RTTI_PROPERTY("Adapters",		&nap::CVVideoCapture::mAdapters,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVVideoCapture::~CVVideoCapture()			{ }


	bool CVVideoCapture::grab(CVFrame& target)
	{
		// Check if a new frame is available
		if (!mFrameAvailable)
			return false;

		// Copy last captured frame using a deep copy.
		// Again, the deep copy is necessary because a weak copy allows
		// for the data to be updated by the capture loop whilst still processing on another thread.
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		mCaptureMat[0].copyTo(target);
		mFrameAvailable = false;
		return true;
	}
	

	void CVVideoCapture::capture()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mCaptureFrame = true;
		}
		mCaptureCondition.notify_one();
	}


	bool CVVideoCapture::start(utility::ErrorState& errorState)
	{
		// Initialize property map
		mPropertyMap.reserve(mAdapters.size());
		for (auto& adapter : mAdapters)
		{
			// Create properties
			mPropertyMap[adapter.get()] = {};
			adapter->mParent = this;
			
			// Open adapter
			if (!adapter->open(errorState))
				return false;
		}

		// Start capture task
		mStopCapturing = false;
		mCaptureTask = std::async(std::launch::async, std::bind(&CVVideoCapture::captureTask, this));
		return true;
	}


	void CVVideoCapture::setProperty(CVAdapter& adapter, cv::VideoCaptureProperties propID, double value)
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			auto it = mPropertyMap.find(&adapter);
			if (it == mPropertyMap.end())
			{
				nap::Logger::warn("%s: unknown CVAdapter: %s", this->mID.c_str(), adapter.mID.c_str());
				return;
			}
			(*it).second[propID] = value;
		}
		mCaptureCondition.notify_one();
	}


	void CVVideoCapture::stop()
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

		// Notify all adapters
		for (auto& adapter : mAdapters)
		{
			adapter->close();
			adapter->mParent = nullptr;
		}

		// Clear all properties
		mPropertyMap.clear();
	}


	void CVVideoCapture::captureTask()
	{
		// Wait for playback to be enabled, a new frame request is issued or request to stop is made
		// Exit loop immediately when a stop is requested. Otherwise process next frame
		CVFrameEvent frame_event;
		frame_event.reserve(mAdapters.size());
		std::unordered_map<CVAdapter*, PropertyMap> properties;
		std::vector<nap::CVAdapter*> capture_adapters;
		bool set_properties = false;

		// Used to calculate framerate over time
		std::array<double, 20> mTicks;
		double mTicksum = 0;
		uint32 mTickIdx = 0;
		SystemTimer timer;
		timer.start();
		int previous_time = 0;

		while(!mStopCapturing)
		{
			nap::Logger::info("loop milli: %d", timer.getMillis());
			timer.reset();
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
				properties = mPropertyMap;
				for (auto& prop : mPropertyMap)
				{
					prop.second.clear();
				}

				// Reset this flag immediately, ensures new requests are forwarded immediately.
				mCaptureFrame = false;
			}

			// Apply properties for every adapter and grab next frame
			// Store adapters that are ready to be captured
			capture_adapters.clear();
			for (auto& adapter : properties)
			{
				nap::CVAdapter* cur_adapter = adapter.first;
				for (const auto& prop : adapter.second)
				{
					if (!cur_adapter->getCaptureDevice().set(prop.first, prop.second))
					{
						nap::Logger::warn("%s unable to set property: %s to: %.02f", cur_adapter->mID.c_str(), prop.first, prop.second);
					}
				}
				
				if (!cur_adapter->getCaptureDevice().grab())
				{
					nap::Logger::warn("%s: failed to grab frame from %s. Device disconnected or end of stream.", mID.c_str(), cur_adapter->mID.c_str());
					continue;
				}
				capture_adapters.emplace_back(cur_adapter);
			}

			// Now retrieve frame (heaviest operation)
			nap::utility::ErrorState grab_error;

			frame_event.clear();
			for (auto& adapter : capture_adapters)
			{
				frame_event.addFrame(adapter->retrieve(grab_error));
				if (frame_event.lastFrame().empty())
				{
					nap::Logger::error("%s: failed to decode frame", mID.c_str());
					frame_event.popFrame();
					continue;
				}
				adapter->copied();
			}

			// No frames captured
			if (frame_event.empty())
				continue;

			// Deep copy the captured frame to our storage matrix.
			// This updates the data of our storage container and ensures the same dimensionality.
			// We need to perform a deep-copy because if we choose to use a shallow copy, 
			// by the time the frame is grabbed the data 'mCaptureMat' points to could have changed, 
			// as it references the same data as in the event. And the process loop already started.
			// Performing a deep_copy ensures that when the data is grabbed it will contain the latest full processed frame.
			//
			// Alternatively, we could make the capture variable local to this loop, but that creates
			// more overhead than the copy below.
			{
				std::lock_guard<std::mutex> lock(mCaptureMutex);
				frame_event.copyTo(mCaptureMat);
			}

			// TODO: Send Signal!

			// New frame is available
			mFrameAvailable = true;
		}
	}
}