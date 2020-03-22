/// local includes
#include "cvservice.h"
#include "cvcapturedevice.h"
#include "cvevent.h"
#include "cvcapturecomponent.h"

// external includes
#include <opencv2/core/ocl.hpp>
#include <nap/logger.h>
#include <unordered_map>

RTTI_BEGIN_CLASS(nap::CVServiceConfiguration)
	RTTI_PROPERTY("ThreadCount", &nap::CVServiceConfiguration::mThreadCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	CVService::CVService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	void CVService::setThreadCount(int count)
	{
		cv::setNumThreads(count);
	}


	int CVService::getThreadCount() const
	{
		return cv::getNumThreads();
	}


	bool CVService::init(nap::utility::ErrorState& errorState)
	{
		int thread_count = getConfiguration<CVServiceConfiguration>()->mThreadCount;
		cv::setNumThreads(thread_count);
		return true;
	}


	void CVService::update(double deltaTime)
	{
		// Iterate over every capture device, if any capture component
		// is interested in frames from a particular capture device grab that 
		// frame when new and forward to that component. By default no deep
		// copy operation is performed, components should handle the copy themselves.
		for (auto& device : mCaptureDevices)
		{
			// Check if a new frame is available
			if(!device->newFrame())
				continue;

			// Find relevant components for device to forward frame to
			std::vector<CVCaptureComponentInstance*> valid_components;
			valid_components.reserve(mCaptureComponents.size());
			for (auto& component : mCaptureComponents)
			{
				// Component accepts frame from specific device
				if (component->mDevice == device)
					valid_components.emplace_back(component);
			}

			// No matching components
			if(valid_components.empty())
				continue;

			// Consume frame and forward to interested components
			CVFrameEvent capture_frame;
			device->grab(capture_frame, false);
			for (auto& component : valid_components)
			{
				component->trigger(capture_frame);
			}
		}
	}


	void CVService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<CVCaptureDeviceObjectCreator>(*this));
	}


	void CVService::registerCaptureDevice(CVCaptureDevice& device)
	{
		mCaptureDevices.emplace_back(&device);
	}


	void CVService::removeCaptureDevice(CVCaptureDevice& device)
	{
		auto found_it = std::find_if(mCaptureDevices.begin(), mCaptureDevices.end(), [&](const auto& it)
		{
			return it == &device;
		});
		assert(found_it != mCaptureDevices.end());
		mCaptureDevices.erase(found_it);
	}


	void CVService::registerCaptureComponent(CVCaptureComponentInstance& input)
	{
		mCaptureComponents.emplace_back(&input);
	}

	
	void CVService::removeCaptureComponent(CVCaptureComponentInstance& input)
	{
		auto found_it = std::find_if(mCaptureComponents.begin(), mCaptureComponents.end(), [&](const auto& it)
		{
			return it == &input;
		});
		assert(found_it != mCaptureComponents.end());
		mCaptureComponents.erase(found_it);
	}
}

