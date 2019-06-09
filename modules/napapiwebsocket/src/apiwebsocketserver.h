#pragma once

// Local Includes
#include "websocketserver.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	class APIWebSocketService;

	/**
	 * Allows for receiving and responding to nap API messages over a web-socket. Implements the IWebSocketServer interface.
	 */
	class NAPAPI APIWebSocketServer : public IWebSocketServer
	{
		RTTI_ENABLE(IWebSocketServer)
	public:

		// 'NAP:MESSAGE': The string every API web-socket request should be prefixed with
		static const std::string apiMessageHeaderName;

		// Destructor
		virtual ~APIWebSocketServer();

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

		bool mVerbose = true;				///< Property: 'Verbose' log message extraction failure information
		bool mForward = true;				///< Property: 'ForwardMessage' if a message is forwarded to a web-socket component, next to the API service.

	private:
		APIWebSocketService* mService = nullptr;

		/**
		 * Sends an error reply to the specified connection. When verbose is turned on a warning is generated.
		 * Every error reply is prefixed with: 'ERROR'.
		 * @param connection client connection
		 * @param error the error to send.
		 */
		void sendErrorReply(const WebSocketConnection& connection, nap::utility::ErrorState& error);

		// Called when the api server receives a new message
		void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message);
		nap::Slot<const WebSocketConnection&, const WebSocketMessage&> mMessageReceived;

		void onConnectionOpened(const WebSocketConnection& connection);
		nap::Slot<const WebSocketConnection&> mConnectionOpened;

		void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason);
		nap::Slot<const WebSocketConnection&, int, const std::string&> mConnectionClosed;

		void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason);
		nap::Slot<const WebSocketConnection&, int, const std::string&> mConnectionFailed;
	};

	// Object creator used for constructing the the api web-socket server
	using APIWebSocketServerObjectCreator = rtti::ObjectCreator<APIWebSocketServer, APIWebSocketService>;
}
