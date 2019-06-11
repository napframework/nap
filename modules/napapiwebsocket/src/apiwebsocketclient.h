#pragma once

// Local Includes
#include "apiwebsocketevent.h"
#include "apiwebsocketutils.h"

// External Includes
#include <websocketclient.h>

namespace nap
{
	// Forward Declares
	class APIWebSocketService;
	class APIService;

	/**
	 * With this client you can send NAP API events to a server. When the server issues
	 * a reply the message can be interpreted as a NAP API event and is, when accepted, forwarded to the API service.
	 * This allows you to use the NAP API system to send and respond to server requests.
	 * 
	 * A reply is accepted when your application exposes a signature (method) that matches the signature of
	 * the received message. When accepted, the right callback in your application is automatically triggered by a 
	 * nap::APIComponentInstance. Look at the documentation of the nap::APIComponentInstance and nap::APICallBack
	 * for more information.
	 *
	 * For example, to send a message in the form of an api request to a server:
	 *
	 *		// Create request
	 *		nap::APIEventPtr server_request = std::make_unique<nap::APIEvent>("getMotorSpeed");
	 *
	 *		// Add argument
	 *		server_request->addArgument<APIInt>("number", 1);
	 *
	 *		// Send
	 *		nap::utility::ErrorState error;
	 *		if (!this->send(std::move(server_request), error))
	 *		{
	 *			nap::Logger::error(error.toString());
	 *		}
	 *
	 * You can control what the client does when it a receives a connection update or message from the server.
	 * By default messages are NOT converted into API events, only into web-socket events. 
	 * By changing the 'Mode' to 'APIEvent' or 'Both' received messages are converted into API events 
	 * and forwarded to your application (if the system accepts it). When only 'APIEvent' is selected 
	 * no web-socket events are created, when 'Both' is selected the client will create both api and web-socket events.
	 * To catch api-events use a nap::APIComponentInstance, to catch web-socket events use a nap::WebSocketComponent.
	 * 
	 * NOTE: When the 'Mode' is set to 'APIEvent' NO connection updates (open, close and failed) are created.
	 * When 'Verbose' is turned on the client will issue warnings if a message can't be converted into an api event.
	 */

	class NAPAPI APIWebSocketClient : public IWebSocketClient
	{
		RTTI_ENABLE(IWebSocketClient)
	public:
		/**
		 * Constructor
		 * @param service handle to the api web-socket service.
		 */
		APIWebSocketClient(APIWebSocketService& service);

		// Destructor
		virtual ~APIWebSocketClient();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends a message in the form of an API event to the server.
		 * For example:
		 *
		 *		// Create request
		 *		nap::APIEventPtr server_request = std::make_unique<nap::APIEvent>("getMotorSpeed");
		 *
		 *		// Add argument
		 *		server_request->addArgument<APIInt>("number", 1);
		 *
		 *		// Send
		 *		nap::utility::ErrorState error;
		 *		if (!this->send(std::move(server_request), error))
		 *		{
		 *			nap::Logger::error(error.toString());
		 *		}
		 *
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

		EWebSocketForwardMode mMode = EWebSocketForwardMode::WebSocketEvent;	///< Property: 'Mode' web-socket event translation and forward mode.
		bool mVerbose = true;													///< Property: 'Verbose' log server message to api-event conversion failures.

	protected:
		// Called by web-socket client endpoint when the connection is opened
		virtual void onConnectionOpened() override;
		// Called by web-socket client endpoint when the connection is closed
		virtual void onConnectionClosed(int code, const std::string& reason) override;
		// Called by web-socket client endpoint when the connection failed to establish
		virtual void onConnectionFailed(int code, const std::string& reason) override;
		// Called by web-socket client endpoint when a new message is received
		virtual void onMessageReceived(const WebSocketMessage& msg) override;

	private:
		APIService* mAPIService = nullptr;	///< Handle to the api service.
	};

	// Object creator used for constructing the api web-socket client
	using APIWebSocketClientObjectCreator = rtti::ObjectCreator<APIWebSocketClient, APIWebSocketService>;
}
