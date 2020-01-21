/// local includes
#include "cvservice.h"
#include "cvcapturedevice.h"
#include "cvevent.h"

// external includes
#include <opencv2/core/ocl.hpp>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	CVService::CVService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	void CVService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}


	bool CVService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void CVService::update(double deltaTime)
	{
	}


	void CVService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<CVCaptureDeviceObjectCreator>(*this));
	}


	void CVService::registerCaptureDevice(CVCaptureDevice& device)
	{
		device.mFrameCaptured.connect(mFrameAvailable);
	}


	void CVService::removeCaptureDevice(CVCaptureDevice& device)
	{
		device.mFrameCaptured.disconnect(mFrameAvailable);
	}


	void CVService::onFrameCaptured(const CVFrameEvent&)
	{
		
	}
}

