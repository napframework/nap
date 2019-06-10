#pragma once

// Local Includes
#include "websocketevent.h"

// External Includes
#include <nap/resource.h>
#include <queue>
#include <mutex>

namespace nap
{
	class WebSocketService;

	/**
	 * websocketinterface
	 */
	class NAPAPI WebSocketInterface : public Resource
	{
		friend class WebSocketService;
		RTTI_ENABLE(Resource)
	public:
		WebSocketInterface(WebSocketService& service);

		virtual ~WebSocketInterface();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns the interface as an interface of type T.
		 * The interface is either a server or client, for example: as<nap::WebSocketServer>();
		 * Asserts if the interface isn't of type T.
		 * @return The web-socket interface associated with this event.
		 */
		template<typename T>
		T& as();

		/**
		 * Returns the interface as an interface of type T.
		 * The interface is either a server or client, for example: as<nap::WebSocketServer>();
		 * Asserts if the interface isn't of type T.
		 * @return The web-socket interface associated with this event.
		 */
		template<typename T>
		const T& as();

	protected:
		/**
		 * Called when the end point receives a new event.
		 * Adds the event to the list of events to be processed on the main thread.
		 * @param newEvent the web-socket event.
		 */
		void addEvent(WebSocketEventPtr newEvent);

		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		// Handle to the web socket service
		WebSocketService* mService = nullptr;

	private:
		/**
		 * Consumes all received web-socket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred web-socket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);
	};

	
	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	const T& nap::WebSocketInterface::as()
	{
		T* cast_interface = rtti_cast<T>(this);
		assert(cast_interface != nullptr);
		return *cast_interface;
	}


	template<typename T>
	T& nap::WebSocketInterface::as()
	{
		T* cast_interface = rtti_cast<T>(this);
		assert(cast_interface != nullptr);
		return *cast_interface;
	}

}
