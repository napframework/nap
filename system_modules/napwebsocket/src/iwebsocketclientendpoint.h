/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"

// External Includes
#include <nap/device.h>

namespace nap
{
	// Forward Declares
	class IWebSocketClient;

	/**
	 * Web socket client endpoint interface.
	 * Manages a list of client-server connections and acts as the main portal for the client to the server.
	 * Every web-socket client receives connection updates and messages from an endpoint.
	 * The endpoint is a device that can be started and stopped. 
	 *
	 * When stopped all active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. Typically an application
	 * has only one endpoint. Multiple clients can reference the same endpoint.
	 * Every connection update and message is forwarded to the client from a background thread.
	 * To receive connection updates and messages a client must be dervied from nap::IWebSocketClient.
	 *
	 * Note that depending on your operating system you might have to run the application as administrator
	 * to open a web-socket.
	 */
	class NAPAPI IWebSocketClientEndPoint : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:

		/**
		 * Sends a message to a server.
		 * @param connection the client connection to the server.
		 * @param message the message to send.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully
		 */
		virtual bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * Sends a message using the given payload and opcode to a server.
		 * @param connection the client connection to the server.
		 * @param payload the message buffer.
		 * @param length buffer size in bytes.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully.
		 */
		virtual bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	protected:
		/**
		 * Connects a nap client to a server. The new connection is managed by this endpoint.
		 * The client is added to the list of internally managed clients.
		 * @param client the client to register
		 * @param error contains the error is registration fails.
		 * @return if the client was registered successfully
		 */
		virtual bool registerClient(IWebSocketClient& client, utility::ErrorState& error) = 0;

		/**
		 * Removes a client (resource) from the list of actively managed connection.
		 * If the client connection is currently open it will be closed.
		 * Asserts if the client isn't part of the system or can't be removed.
		 * @param client the client to remove.
		 */
		virtual void unregisterClient(const IWebSocketClient& client) = 0;
	};
}
