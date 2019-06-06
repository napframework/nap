#pragma once

// Local Includes
#include "apievent.h"

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>

namespace nap
{
	class APIService;

	/**
	 * Interface for all api dispatcher types. Allows for customizing api event dispatch behavior.
	 * An api dispatcher forwards an api event to an external environment.
	 * Every dispatcher is automatically registered with the api service. When an event is dispatched
	 * (through the api service) every registered dispatcher is invoked if it accepts the api event. 
	 * The event that is dispatched must be derived from APIEvent. Override the getEventType() method
	 * to specify which type of event the dispatcher accepts and onDispatch() to forward the  
	 * event to an external application environment.
	 */
	class NAPAPI IAPIDispatcher : public Resource
	{
		friend class APIService;
		RTTI_ENABLE(Resource)
	public:

		// Destructor
		virtual ~IAPIDispatcher();

		// No default constructor
		IAPIDispatcher() = delete;

		/**
		 * Every dispatcher needs to be created with a handle to the api service
		 * @param service handle to the api service
		 */
		IAPIDispatcher(APIService& sevice);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Should be implemented by all api dispatcher types.
		 * Only api events derived from the returned type will be forwarded to the dispatcher.
		 * @return the event type that can be dispatched. Should be derived from APIEvent.
		 */
		virtual rtti::TypeInfo getEventType() const = 0;

	protected:
		/**
		 * Called by the api service after receiving a new event to dispatch.
		 * Needs to be implemented by derived classes.
		 * @param apiEvent the event to dispatch
		 * @param error contains the error if dispatching fails
		 * @return if the event has been dispatched.
		 */
		virtual bool onDispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error) = 0;

		APIService* mService = nullptr;

	private:
		/**
		 * Called by the api service. Calls onDispatch() internally.
		 */
		bool dispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error);
	};


	//////////////////////////////////////////////////////////////////////////
	// Default event dispatcher
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Default API dispatcher. When this dispatcher receives an api event it triggers 
	 * the 'eventDispatched' signal of the api service. All api events are accepted by this dispatcher.
	 * When declared in JSON, an external environment that listens
	 * to the 'eventDispatched' signal, automatically receives the dispatched api events.
	 */
	class NAPAPI APIDispatcher : public IAPIDispatcher
	{
		RTTI_ENABLE(IAPIDispatcher)
	public:
		
		/**
		 * Every dispatcher needs to be created with a handle to the api service
		 * @param service handle to the api service
		 */
		APIDispatcher(APIService& service);

		/**
		 * Initializes the api dispatcher
		 * @param errorState contains the error message if initialization failed
		 * @return if initialization succeeded. 
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Accepts all api events! Triggers the dispatch signal of the service
		 * @return accepts all api events.
		 */
		virtual rtti::TypeInfo getEventType() const override			{ return RTTI_OF(nap::APIEvent); }

		/**
		 * Called when the api service receives a new api event. Avery api event is accepted!
		 * @param apiEvent the event to dispatch.
		 * @param error contains the error if dispatching failed.
		 * @return if the event has been dispatched.
		 */
		virtual bool onDispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error);
	};

	// Object creator used for constructing the api dispatcher
	using APIDispatcherObjectCreator = rtti::ObjectCreator<APIDispatcher, APIService>;
}
