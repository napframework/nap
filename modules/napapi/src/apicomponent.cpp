#include "apicomponent.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <entity.h>

// nap::apicomponent run time class definition 
RTTI_BEGIN_CLASS(nap::APIComponent)
	RTTI_PROPERTY("Deferred",	&nap::APIComponent::mDeferred,	 nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Methods",	&nap::APIComponent::mSignatures, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::apicomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	APIComponentInstance::~APIComponentInstance()
	{
		if (mAPIService != nullptr)
		{
			mAPIService->removeAPIComponent(*this);
		}
	}


	bool APIComponentInstance::init(utility::ErrorState& errorState)
	{
		// Store api service and register
		mAPIService = getEntityInstance()->getCore()->getService<nap::APIService>();
		assert(mAPIService != nullptr);
		mAPIService->registerAPIComponent(*this);

		// Copy over list of accepted calls
		std::vector<ResourcePtr<APISignature>>& methods = getComponent<APIComponent>()->mSignatures;
		for (const auto& method : methods)
			mSignatures.emplace(std::make_pair(method->mID, method.get()));

		// Store if we need to execute deferred
		mDeferred = getComponent<APIComponent>()->mDeferred;
		
		return true;
	}


	nap::APISignature* APIComponentInstance::findSignature(const std::string& id)
	{
		// First perform quick lookup
		const auto it = mSignatures.find(id);
		if (it == mSignatures.end())
			return nullptr;
		return it->second;
	}


	bool APIComponentInstance::accepts(const APIEvent& apiEvent) const
	{
		// First perform quick lookup
		const auto it = mSignatures.find(apiEvent.getID());
		if (it == mSignatures.end())
			return false;

		// Check if this signature accepts the event, ie:
		// The number of arguments and type of arguments in order must match
		return apiEvent.matches(*it->second);
	}


	void APIComponentInstance::update(double deltaTime)
	{
		// Don't do anything when we are handling events in deferred mode.
		if (!mDeferred)
			return;

		// Copy api events thread safe
		std::queue<APIEventPtr> out_events;
		{
			std::lock_guard<std::mutex> lock(mCallMutex);
			out_events.swap(mAPIEvents);
			std::queue<APIEventPtr> empty_queue;
			mAPIEvents.swap(empty_queue);
		}

		// Process api events
		while (!out_events.empty())
		{
			messageReceived(*out_events.front());
			out_events.pop();
		}
	}


	void APIComponentInstance::trigger(APIEventPtr apiEvent)
	{
		// Make sure the call is accepted
		assert(accepts(*apiEvent));

		// If we defer the call, add it and run over it later
		if (mDeferred)
		{
			std::lock_guard<std::mutex> lock_guard(mCallMutex);
			mAPIEvents.push(std::move(apiEvent));
			return;
		}

		// Forward to potential listeners
		messageReceived(*apiEvent);
	}
}