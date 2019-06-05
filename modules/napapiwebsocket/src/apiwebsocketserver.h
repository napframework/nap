#pragma once

// Local Includes
#include "websocketserver.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	class APIWebSocketService;

	/**
	 * Allows for receiving and responding to nap API events over a web-socket. Implements the IWebSocketServer interface.
	 */
	class NAPAPI APIWebSocketServer : public IWebSocketServer
	{
		RTTI_ENABLE(IWebSocketServer)
	public:
		virtual ~APIWebSocketServer();

		// No default constructor
		APIWebSocketServer() = delete;

		/**
		 * Constructor.
		 * @param service the api websocket service.
		 */
		APIWebSocketServer(APIWebSocketService& service);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		APIWebSocketService* mService = nullptr;

		// Called when the api server receives a new message
		void onMessageReceived(WebSocketConnection connection, WebSocketMessage message);
		nap::Slot<WebSocketConnection, WebSocketMessage> mMessageReceived;
	};

	// Object creator used for constructing the the api web-socket server
	using APIWebSocketServerObjectCreator = rtti::ObjectCreator<APIWebSocketServer, APIWebSocketService>;
}
