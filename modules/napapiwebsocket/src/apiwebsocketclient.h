#pragma once

// Local Includes
#include "apiwebsocketevent.h"

// External Includes
#include <websocketclient.h>

namespace nap
{
	class NAPAPI APIWebSocketClient : public IWebSocketClient
	{
		RTTI_ENABLE(IWebSocketClient)
	public:
		// Constructor used by factory
		APIWebSocketClient(WebSocketService& service);

		// Destructor
		virtual ~APIWebSocketClient();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends a message in the form of an event to the server.
		 * @param apiEvent event that contains the message to send.
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(APIEventPtr apiEvent, utility::ErrorState& error);

		/**
		 * Tries to converts a web-socket (text) message into one or multiple individual nap api events.
		 * The message should be the result of a previous query to a NAP server.
		 * @param message the received web-socket (text) message that contains individual nap::APIMessage objects.
		 * @param outEvents the result of the extraction process.
		 * @error contains the error if conversion fails
		 * @return if conversion succeeded.
		 */
		bool convert(const WebSocketMessage& message, std::vector<APIEventPtr>& outEvents, utility::ErrorState& error);

	protected:
		virtual void onConnectionOpened() override;
		virtual void onConnectionClosed(int code, const std::string& reason) override;
		virtual void onConnectionFailed(int code, const std::string& reason) override;
		virtual void onMessageReceived(const WebSocketMessage& msg) override;
	};

	// Object creator used for constructing the websocket client
	using WebSocketClientObjectCreator = rtti::ObjectCreator<WebSocketClient, WebSocketService>;
}
