#pragma once

// External Includes
#include <component.h>
#include <queue>
#include <mutex>

// Local Includes
#include "apiservice.h"
#include "apievent.h"

namespace nap
{
	class APIComponentInstance;

	/**
	 * Receives APIEvents from the APIService.
	 * Populate the 'CallFilter' with names of API events you wish to receive at run-time.
	 */
	class NAPAPI APIComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(APIComponent, APIComponentInstance)
	public:
		std::vector<std::string> mCallFilter;		///< Property: 'CallFilter' all (exact) names of API calls (actions) accepted by this component.
		bool mDeferred = false;						///< Property: 'Deferred' if calls are executed immediately or deferred, ie: when the component is updated.
	};


	/**
	 * Receives APIEvents from the APIService and executes them immediately or deferred.
	 * An event is received when an external application calls into to API service to perform a task.
	 * This component either directly executes the call or waits until update is called, based on the deferred toggle.
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
		 * Checks to see if a specific API call is accepted by this component.
		 * @param action the name of the call to accept
		 * @return if a specific API call is accepted by this component	
		 */
		bool accepts(const std::string& action) const;
		
		/**
		 * Executes all deferred calls
		 * @param deltaTime time in between calls in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		// Pointer to the API service
		APIService* mAPIService = nullptr;

		// All accepted names of individual calls
		std::unordered_set<std::string> mCallFilter;

		// Mutex associated with setting / getting calls
		std::mutex	mCallMutex;

		/**
		 * Called by the API service if this component accepts the call.
		 * When accepted this component owns the event.
		 * @param apiEvent event that is accepted and forwarded.
		 * @param error contains the error if the call couldn't be handled correctly.
		 */
		bool call(APICallEventPtr apiEvent, nap::utility::ErrorState& error);

		bool mDeferred = false;					///< if calls are executed deferred, ie: when the component is updated.
		std::queue<APICallEventPtr> mCalls;		///< all calls that need to be executed deferred
	};
}
