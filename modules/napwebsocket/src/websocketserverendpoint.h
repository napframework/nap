#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketmessage.h"

// External Includes
#include <memory.h>
#include <future>
#include <utility/errorstate.h>
#include <nap/device.h>
#include <nap/signalslot.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Server endpoint role.
	 * Creates and manages a connection with the server web socket endpoint.
	 * Call 'start' to start listening and accepting messages.
	 * Call 'stop' to close all active connections and stop listening. A call to open is non blocking.
	 * Install handlers to receive messages at run-time. Note that the messages are received on a different thread!
	 */
	class WebSocketServerEndPoint : public Device
	{
		RTTI_ENABLE(Device)
	public:
		// default constructor
		WebSocketServerEndPoint();

		/**
		 * Stops the end point from running
		 */
		~WebSocketServerEndPoint();

		/**
		 * Initializes the server end point
		 * @param error contains the error when initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Opens the port and starts the run loop.
		 * @param error contains the error if opening failed
		 * @return if the port could be opened
		 */
		virtual bool start(nap::utility::ErrorState& error) override;

		/**
		 * @return if the current end point is open and running
		 */
		bool isOpen() const													{ return mOpen; }

		/**
		 * Stops the end-point from running, all active connections are closed.
		 */
		virtual void stop() override;

		/**
		 * Sends a message to the incoming connection
		 * @param message the message to send
		 * @param connection client to send message to
		 * @param message the original client message
		 */
		void send(const std::string& message, wspp::ConnectionHandle connection, wspp::OpCode opCode);

		int mPort = 80;														///< Property: "Port" to open and listen to for messages.
		bool mLogConnectionUpdates = true;									///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;	///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

		nap::Signal<WebSocketMessage> messageReceived;						///< Signal emitted when a new message is received
		nap::Signal<WebSocketConnection> connectionOpened;					///< Signal emitted when a new connection with a client is opened
		nap::Signal<WebSocketConnection> connectionClosed;					///< Signal emitted when a connection with a client is closed
		nap::Signal<WebSocketConnection> connectionFailed;					///< Signal emitted when a connection with a client failed to establish

	private:
		wspp::ServerEndPoint mEndPoint;										///< The websocketpp server end-point
		uint32 mLogLevel = 0;												///< Converted library log level
		uint32 mAccessLogLevel = 0;											///< Log client / server connection data
		std::future<void> mServerTask;										///< The background server thread
		bool mOpen = false;													///< If connection is open

		/**
		 * Runs the end point in a background thread until stopped.
		 */
		void run();

		/**
		 * Called when a new connection is made
		 */
		void onConnectionOpened(wspp::ConnectionHandle connection);

		/**
		 * Called when a collection is closed
		 */
		void onConnectionClosed(wspp::ConnectionHandle connection);

		/**
		 * Called when a failed connection attempt is made
		 */
		void onConnectionFailed(wspp::ConnectionHandle connection);

		/**
		 * Called when a new message is received
		 */
		void onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg);
	};
}
