/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apicomponent.h"
#include "apiutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIService, "Offers a C-Style interface that can be used to send and receive data from a running NAP application")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	APIService::APIService(ServiceConfiguration* configuration) : Service(configuration)
	{
	}


	APIService::~APIService()
	{
	}


	bool APIService::sendFloat(const char* id, float value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIFloat>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendString(const char* id, const char* value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIString>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendInt(const char* id, int value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIInt>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByte(const char* id, nap::uint8 value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIByte>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendBool(const char* id, bool value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIBool>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendLong(const char* id, int64_t value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APILong>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendChar(const char* id, char value, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIChar>("data", value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::send(const char* id, utility::ErrorState* error)
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendEvent(APIEventPtr apiEvent, utility::ErrorState* error)
	{
		return forward(std::move(apiEvent), *error);
	}


	bool APIService::sendMessage(const char* json, utility::ErrorState* error)
	{
		// Get factory
		auto& factory = getCore().getResourceManager()->getFactory();

		// Extract messages
		nap::rtti::DeserializeResult result;
		std::vector<nap::APIMessage*> messages;
		if (!extractMessages(json, result, factory, messages, *error))
			return false;

		// Iterate over every message, extract arguments (APIFloat etc.) and send as new event.
		// Note that we make a copy of the argument because the original arguments are owned by the de-serialized result
		bool succeeded = true;
		for (auto& message : messages)
		{
			// Forward, keep track of result
			if (!forward(message->toEvent<APIEvent>(), *error))
				succeeded = false;
		}

		// Notify user if all messages were send successfully.
		if (!error->check(succeeded, "Unable to forward all JSON messages"))
			return false;
		return true;
	}


	bool APIService::sendIntArray(const char* id, int* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<int> values(length);
		std::memcpy(values.data(), array, sizeof(int)*length);

		// Move construct argument and send.
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIIntArray>("data", std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendFloatArray(const char* id, float* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<float> values(length);
		std::memcpy(values.data(), array, sizeof(float)*length);

		// Move construct argument and send.
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIFloatArray>("data", std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByteArray(const char* id, uint8_t* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<uint8_t> values(length);
		std::memcpy(values.data(), array, sizeof(uint8_t)*length);

		// Move construct argument and send.
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIByteArray>("data", std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendStringArray(const char* id, const char** array, int length, utility::ErrorState* error)
	{
		// Create array of strings out of raw data
		std::vector<std::string> values;
		values.reserve(length);
		for(int i=0; i<length; i++)
			values.emplace_back(std::string(array[i]));

		// Move construct argument and send.
		APIEventPtr ptr = std::make_unique<APIEvent>(id);
		ptr->addArgument<APIStringArray>("data", std::move(values));
		return forward(std::move(ptr), *error);
	}


	void APIService::dispatchEvent(nap::APIEventPtr apiEvent)
	{
		eventDispatched.trigger(*apiEvent);
	}

	
	void APIService::processEvents()
	{
		// Consume all given events
		std::queue<APIEventPtr> api_events;
		consumeEvents(api_events);

		// Forward to api components for processing
		while (!api_events.empty())
		{
			// Get event
			APIEvent& current_event = *api_events.front();

			// Iterate over every api component
			// Find a matching callback and forward
			for (auto& api_comp : mAPIComponents)
			{
				if (api_comp->accepts(current_event))
					api_comp->trigger(current_event);
			}
			api_events.pop();
		}
	}


	void APIService::registerAPIComponent(APIComponentInstance& apicomponent)
	{
		std::lock_guard<std::mutex> lock(mComponentMutex);
		mAPIComponents.emplace_back(&apicomponent);
	}


	void APIService::removeAPIComponent(APIComponentInstance& apicomponent)
	{
		std::lock_guard<std::mutex> lock(mComponentMutex);
		auto found_it = std::find_if(mAPIComponents.begin(), mAPIComponents.end(), [&](const auto& it)
		{
			return it == &apicomponent;
		});
		assert(found_it != mAPIComponents.end());
		mAPIComponents.erase(found_it);
	}


	bool APIService::forward(APIEventPtr apiEvent, utility::ErrorState& error)
	{
		// Make sure components don't get pulled / updated while forwarding calls
		std::lock_guard<std::mutex> lock(mComponentMutex);

		// Iterate over every api component
		// Find a matching callback and forward
		for (auto& api_comp : mAPIComponents)
		{
			// Check signature, if found and arguments match forward
			const APISignature* signature = api_comp->findSignature(apiEvent->getName());
			if (signature != nullptr)
			{
				if (!error.check(apiEvent->matches(*signature), "Signature mismatch for call: %s, component: %s", signature->mID.c_str(), 
					api_comp->getComponent<APIComponent>()->mID.c_str()))
					return false;
				
				// Add event safely
				std::lock_guard<std::mutex> lock_guard(mEventMutex);
				mAPIEvents.push(std::move(apiEvent));
				return true;
			}		
		}

		// No component accepted the call!
		return error.check(false, "%s: No matching signature found for: %s", this->get_type().get_name().data(), apiEvent->getName().c_str());
	}


	void APIService::consumeEvents(std::queue<APIEventPtr>& outEvents)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		outEvents.swap(mAPIEvents);
		std::queue<APIEventPtr> empty_queue;
		mAPIEvents.swap(empty_queue);
	}


	bool APIService::init(utility::ErrorState& error)
	{
		return true;
	}


	void APIService::shutdown()
	{

	}


	void APIService::update(double deltaTime)
	{
		processEvents();
	}
}
