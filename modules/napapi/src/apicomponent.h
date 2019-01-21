#pragma once

// External Includes
#include <component.h>
#include <queue>
#include <mutex>
#include <nap/signalslot.h>

// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apisignature.h"

namespace nap
{
	class APIComponentInstance;

	/**
	 * Receives APIEvents from the APIService. 
	 * Populate the 'Methods' property with signatures of calls you wish to receive at run-time.
	 * Only events that have a matching signature are accepted.
	 */
	class NAPAPI APIComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(APIComponent, APIComponentInstance)
	public:
		std::vector<ResourcePtr<APISignature>> mSignatures;			///< Property: 'Methods' all (exact) names of API calls (ids) accepted by this component.
		bool mDeferred = false;										///< Property: 'Deferred' if calls are executed immediately or deferred, ie: when the component is updated.
	};


	/**
	 * Receives APIEvents from the APIService and executes them immediately or deferred.
	 * An event is received when an external application calls into to API service to perform a specific task.
	 * This component either directly executes the call or waits until update is called, based on the deferred toggle.
	 * Only events with a matching signature are accepted and forwarded. 
	 * 
	 * To receive api events listen to the 'messageReceived' signal, which is triggered when a new api event is received.
	 * When received, you know for sure the values of the events precisely match that of the registered signature.
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
		 * Tries to find a signature with the given name.
		 * @param id name of the signature.
		 * @return found signature, nullptr if not found.
		 */
		APISignature* findSignature(const std::string& id);

		/**
		 * Checks to see if a specific API call is accepted by this component.
		 * @param apiEvent the api event to accept
		 * @return if a specific API call is accepted by this component	
		 */
		bool accepts(const APIEvent& apiEvent) const;
		
		/**
		 * Executes all deferred calls.
		 * @param deltaTime time in between calls in seconds
		 */
		virtual void update(double deltaTime) override;

		Signal<const APIEvent&> messageReceived;				///< Triggered when the component receives an api event

	private:
		// Pointer to the API service
		APIService* mAPIService = nullptr;

		// All accepted names of individual calls
		std::unordered_map<std::string, APISignature*> mSignatures;

		// Mutex associated with setting / getting calls
		std::mutex	mCallMutex;

		/**
		 * Called by the API service when the component accepts the event.
		 * When accepted this component owns the event.
		 * @param apiEvent event that is accepted and forwarded.
		 */
		 void trigger(APIEventPtr apiEvent);

		bool mDeferred = false;					///< if calls are executed deferred, ie: when the component is updated.
		std::queue<APIEventPtr> mAPIEvents;		///< all calls that need to be executed deferred
	};
}
