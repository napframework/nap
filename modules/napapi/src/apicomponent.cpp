#include "apicomponent.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <entity.h>

// nap::apicomponent run time class definition 
RTTI_BEGIN_CLASS(nap::APIComponent)
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
		// Clear all callbacks
		mCallbacks.clear();

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
		
		return true;
	}


	const nap::APISignature* APIComponentInstance::findSignature(const std::string& id) const
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
		const auto it = mSignatures.find(apiEvent.getName());
		if (it == mSignatures.end())
			return false;

		// Check if this signature accepts the event, ie:
		// The number of arguments and type of arguments in order must match
		return apiEvent.matches(*it->second);
	}


	nap::APICallBack& APIComponentInstance::getOrCreateCallback(const APISignature& signature)
	{
		// See if we haven an existing callback
		auto it = mCallbacks.find(signature.mID);
		if (it != mCallbacks.end())
			return *(it->second);

		// Create one otherwise
		assert(findSignature(signature.mID) != nullptr);	///< signature not represented by this component
		std::unique_ptr<APICallBack> new_callback(std::make_unique<APICallBack>(signature));
		APICallBack* call_ptr = new_callback.get();
		mCallbacks.emplace(std::make_pair(signature.mID, std::move(new_callback)));
		return *call_ptr;
	}


	void APIComponentInstance::registerCallback(const APISignature& signature, nap::Slot<const APIEvent&>& slot)
	{
		APICallBack& callback = getOrCreateCallback(signature);
		callback.messageReceived.connect(slot);
	}


	void APIComponentInstance::trigger(const APIEvent& apiEvent)
	{
		// Forward to registered callbacks
		auto it = mCallbacks.find(apiEvent.getName());
		if (it != mCallbacks.end())
			it->second->messageReceived.trigger(apiEvent);

		// Forward to potential listeners
		messageReceived(apiEvent);
	}


	APICallBack::~APICallBack()
	{
	}

}