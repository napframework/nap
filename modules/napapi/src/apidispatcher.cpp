#include "apidispatcher.h"
#include "apiservice.h"

// nap::apidispatcher run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IAPIDispatcher)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIDispatcher)
	RTTI_CONSTRUCTOR(nap::APIService&)
RTTI_END_CLASS



//////////////////////////////////////////////////////////////////////////


namespace nap
{
	IAPIDispatcher::~IAPIDispatcher()			
	{
		mService->deregisterAPIDispatcher(*this);
	}


	IAPIDispatcher::IAPIDispatcher(APIService& service) : mService(&service)
	{
		mService->registerAPIDispatcher(*this);
	}


	bool IAPIDispatcher::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool IAPIDispatcher::dispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error)
	{
		return onDispatch(apiEvent, error);
	}


	APIDispatcher::APIDispatcher(APIService& service) : IAPIDispatcher(service)
	{
	}


	bool APIDispatcher::init(utility::ErrorState& errorState)
	{
		return IAPIDispatcher::init(errorState);
	}


	bool APIDispatcher::onDispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error)
	{
		mService->eventDispatched.trigger(apiEvent);
		return true;
	}

}