/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "websocketserver.h"
#include "apiwebsocketevent.h"
#include "apiwebsocketutils.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	class APIWebSocketService;
	class APIService;

	/**
	 * Allows for receiving and responding to nap api messages over a web-socket. Implements the IWebSocketServer interface.
	 *
	 * When a client sends a JSON formatted text message to the server, the server tries to convert the message into 
	 * a nap api event. When the api event is accepted by your application, it is forwarded to the API service. 
	 * This allows you to use the NAP API system to send and respond to client requests. 
	 * With this server you can also send replies to a client in the form of a nap api event. 
	 *
	 * A client request is accepted when your application exposes a signature (method) that matches the signature of
	 * the received message. When accepted, the right callback in your application is automatically triggered by a
	 * nap::APIComponentInstance. Look at the documentation of the nap::APIComponentInstance and nap::APICallBack
	 * for more information.
	 * 
	 * The message sent by the client must contain at least one JSON formatted nap::APIMessage object, for example:
	 * ~~~~~
	 *		{
	 *			"Objects":
	 *			[
	 *				{
	 *					"Type": "nap::APIMessage",
	 *					"mID": "01",
	 *					"Name": "getMotorSpeed",
	 *					"Arguments":
	 *					[
	 *						{
	 *							"Type": "nap::APIInt",
	 *							"Name": "number",
	 *							"mID": "02",
	 *							"Value": 1
	 *						}
	 *					]
	 *				}
	 *			]
	 *		}
	 * ~~~~~
	 * You can combine multiple nap::APIMessage objects into a single message. Every nap::APIMessage object is converted
	 * into a separate nap api event, before handed over to the running application.
	 * 
	 * Example of a server reply in response to the client request:
	 * ~~~~~{.cpp}
	 *		// Create response, copy over client query uuid.
	 *		APIWebSocketEventPtr reply = std::make_unique<APIEvent>("motorSpeed", client_request->getID());
	 *
	 *		// Add current motor speed value.
	 *		reply->addArgument<APIFloat>("speed", mMotorSpeed);
	 *
	 *		// Send response to client.
	 *		if (!server->send(std::move(reply), client_connection, error))
	 *		{
	 *			nap::Logger::error(error.toString());
	 *		}
	 * ~~~~~
	 * When replying to a nap api message it is advised to copy the unique id of the request. This allows the client to match the request to a server reply.
	 * The client can manually parse the json formatted text or, when making use of a nap::APIWebSocketClient, automatically parse the reply without inspection.
	 * See nap::APIWebSocketConnection for more information.
	 *
	 * You can control what the server does when it a receives a connection update or message from a client.
	 * By default the server tries to convert every message from a client into an api event.
	 * The api event is forwarded to your application (if the system accepts it).
	 * By changing the 'Mode' to 'Both' every message is converted into both an api and web-socket event. 
	 * Both are forwarded to your application. If the message can't be converted into an api event only the 
	 * web-socket event is forwarded. When 'WebSocketEvent' is selected, only web-socket events are created. In this mode
	 * the server is a regular nap::WebSocketServer. To catch api-events use a nap::APIComponentInstance, 
	 * to catch web-socket events use a nap::WebSocketComponent.
	 
	 * NOTE: When the 'Mode' is set to 'APIEvent' NO connection updates (open, close and failed) are created.
	 * When 'Verbose' is turned on the server will issue warnings if a message can't be converted into an api event.
	 * When conversion fails the server always sends an error reply to the client. Every server error reply starts with: "ERROR:", 
	 * together with the reason for failure, for example: "ERROR: MyServer: unable to parse json". 
	 */
	class NAPAPI APIWebSocketServer : public IWebSocketServer
	{
		RTTI_ENABLE(IWebSocketServer)
	public:

		/**
		 * Constructor
		 * @param service handle to the api web-socket service.
		 */
		APIWebSocketServer(APIWebSocketService& service);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Destroys the socket server.
		*/
		virtual void onDestroy() override;

		/**
		 * Sends a message in the form of an api event to a client.
		 * 
		 * For example:
		 *
		 *		// Create response, copy over client query uuid.
		 *		APIWebSocketEventPtr reply = std::make_unique<APIEvent>("motorSpeed", client_request->getID());
		 *
		 *		// Add current motor speed value.
		 *		reply->addArgument<APIFloat>("speed", mMotorSpeed);
		 *
		 *		// Send response to client.
		 *		if (!server->send(std::move(reply), client_connection, error))
		 *		{
		 *			nap::Logger::error(error.toString());
		 *		}
		 *
		 * When replying to a nap api message it is advised to copy the unique id of the request. 
		 * This allows the client to match the request to a server reply.
		 *
		 * @param apiEvent api event to send as a text message to the client.
		 * @param connection client connection handle.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully.
		 */
		bool send(nap::APIEventPtr apiEvent, const WebSocketConnection& connection, utility::ErrorState& error);

		bool mVerbose = true;											///< Property: 'Verbose' log server message to api-event conversion failures.
		EWebSocketForwardMode mMode = EWebSocketForwardMode::APIEvent;	///< Property: 'Mode' web-socket event translation and forward mode

	private:

		APIService* mAPIService = nullptr;

		/**
		 * Sends an error reply to the specified connection. 
		 * When verbose is turned on a warning is generated.
		 * Every error reply is prefixed with: 'ERROR'.
		 * @param connection client connection
		 * @param error the error to send.
		 */
		void sendErrorReply(const WebSocketConnection& connection, nap::utility::ErrorState& error);

		// Called by web-socket server endpoint when a new message is received
		virtual void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message) override;

		// Called by web-socket server endpoint when a client connection opened
		virtual void onConnectionOpened(const WebSocketConnection& connection) override;

		// Called by web-socket server endpoint when a client connection closed
		virtual void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason) override;

		// Called by web-socket server endpoint when a client connection failed to extablish
		virtual void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason) override;
	};

	// Object creator used for constructing the the api web-socket server
	using APIWebSocketServerObjectCreator = rtti::ObjectCreator<APIWebSocketServer, APIWebSocketService>;
}
