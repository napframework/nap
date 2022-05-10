/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <vector>
#include <unordered_map>
#include <component.h>
#include <nap/signalslot.h>

// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apisignature.h"

namespace nap
{
	class APIComponentInstance;
	class APICallBack;

	/**
	 * Receives APIEvents from the APIService.
	 * Populate the 'Signatures' property with signatures of calls you wish to receive at run-time.
	 * Only events that have a matching signature are accepted.
	 */
	class NAPAPI APIComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(APIComponent, APIComponentInstance)
	public:
		std::vector<ResourcePtr<APISignature>> mSignatures;			///< Property: 'Signatures' all (exact) names of API calls (ids) accepted by this component.
	};


	/**
	 * Receives APIEvents from the APIService and forwards the event to all registered callbacks.
	 * An event is received when an external application calls into to API service to perform a specific task.
	 * Only events with a matching signature are accepted and forwarded.
	 *
	 * To receive api events listen to the 'messageReceived' signal, which is triggered when a new api event is received.
	 * When received, you know for sure the values of the events precisely match that of the registered signature.
	 * For a more specific handling of api events you can create a callback for a given API signature and listen to it's messageReceived signal.
	 */
	class NAPAPI APIComponentInstance : public ComponentInstance
	{
		friend class APIService;
		RTTI_ENABLE(ComponentInstance)
	public:
		/**
		 * Constructor
		 */
		APIComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Destructor, removes itself as a listener from the API service.
		 */
		virtual ~APIComponentInstance() override;

		/**
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Tries to find a method signature with the given name.
		 * @param id name of the signature.
		 * @return found signature, nullptr if not found.
		 */
		const APISignature* findSignature(const std::string& id) const;

		/**
		 * Checks to see if a specific API call is accepted by this component.
		 * @param apiEvent the api event to accept
		 * @return if a specific API call is accepted by this component
		 */
		bool accepts(const APIEvent& apiEvent) const;

		/**
		 * Registers a specific callback that is called after receiving a specific api event.
		 * This is a convenience function that takes a slot and binds it to a new or existing callback.
		 * The signature needs to be acquired using findSignature(). A new callback is created if it doesn't exist already.
		 * @param signature the signature to register a callback for, must be acquired using findSignature()
		 * @param slot the function that is called when receiving an event that matches the given signature.
		 */
		void registerCallback(const APISignature& signature, nap::Slot<const APIEvent&>& slot);

		Signal<const APIEvent&> messageReceived;				///< Triggered when the component receives an api event

	private:
		// Pointer to the API service
		APIService* mAPIService = nullptr;

		// All accepted names of individual calls
		std::unordered_map<std::string, APISignature*> mSignatures;

		// All registered callbacks
		std::unordered_map<std::string, std::unique_ptr<APICallBack>> mCallbacks;

		/**
		 * Called by the API service when the component accepts the event.
		 * @param apiEvent event that is accepted and forwarded.
		 */
		 void trigger(const APIEvent& apiEvent);

		 /**
		  * Returns an API callback based on the given signature, a new callback is created if no existing callback is found.
		  * The signature needs to be acquired using findSignature().
		  * You can use this callback to receive api events targeted at a specific api signature.
		  * @param signature the signature (name / arguments) to register a callback for, must be acquired using findSignature()
		  * @return the callback which is triggered when receiving an api event.
		  */
		 APICallBack& getOrCreateCallback(const APISignature& signature);
	};


	//////////////////////////////////////////////////////////////////////////
	// API Callback
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Helper class, allows for binding a callback to an api signature.
	 * This object is created and managed by the APIComponent.
	 * Listen to the messageReceived signal to receive api events that target a specific api signature.
	 * Use APIComponent::getOrCreateCallback() to acquire a callback based on an API signature.
	 */
	class NAPAPI APICallBack final
	{
	public:
		// No default constructor
		APICallBack() = delete;

		// Removes binding
		~APICallBack();

		/**
		* Copy is not allowed
		*/
		APICallBack(APICallBack&) = delete;
		APICallBack& operator=(const APICallBack&) = delete;

		/**
		* Move is not allowed
		*/
		APICallBack(APICallBack&&) = delete;
		APICallBack& operator=(APICallBack&&) = delete;

		/**
		* Creates a callback based on the given api signature
		* @param signature the method signature to create a callback for
		*/
		APICallBack(const APISignature& signature) { }

		Signal<const APIEvent&> messageReceived;	///< called when this callback receives an API message
	};
}
