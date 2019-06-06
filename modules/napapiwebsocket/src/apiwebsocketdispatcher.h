#pragma once

// Local Includes
#include "apiwebsocketevent.h"

// External Includes
#include <apidispatcher.h>
#include <websocketserver.h>

namespace nap
{
	class APIWebSocketService;

	/**
	 * Dispatches an event api event as a message to a web-socket end-point
	 */
	class NAPAPI APIWebSocketDispatcher : public IAPIDispatcher
	{
		RTTI_ENABLE(IAPIDispatcher)
	public:
		virtual ~APIWebSocketDispatcher();

		/**
		 * Every dispatcher needs to be created with a handle to the api service
		 * @param service handle to the api service
		 */
		APIWebSocketDispatcher(APIWebSocketService& service);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		nap::ResourcePtr<nap::IWebSocketServer> mServer;		///< Property: 'Server' the end point that manages the connection.

		/**
		 * Accepts only api web-socket events!
		 * @return type information of APIWebSocketEvent.
		 */
		virtual rtti::TypeInfo getEventType() const override	{ return RTTI_OF(nap::APIWebSocketEvent); }

	protected:

		/**
		* Called when the api service receives a new api event. 
		* Extracts the APIMessage and sends it as a json formatted string to the connection stored in the event.
		* It is important that the linked endpoint manages the connection stored in the event.
		* This call is not threaded! 
		* If the internal conversion operation fails an error is generated and send to the endpoint.
		* @param apiEvent the event to dispatch.
		* @param error contains the error if dispatching failed.
		* @return if the event has been dispatched.
		*/
		virtual bool onDispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error);

	private:
		APIWebSocketService* mAPIWebSocketService = nullptr;

		/**
		 * Sends an error reply to the specified connection.
		 * Every error reply is prefixed with: 'ERROR'.
		 * @param connection client connection
		 * @param error the error to send.
		*/
		void sendErrorReply(const WebSocketConnection& connection, nap::utility::ErrorState& error);
	};

	// Object creator used for constructing the api dispatcher
	using APIWebSocketDispatcherObjectCreator = rtti::ObjectCreator<APIWebSocketDispatcher, APIWebSocketService>;
}
