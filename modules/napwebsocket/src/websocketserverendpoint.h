#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"
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
	class IWebSocketServer;

	/**
	 * Server endpoint role. Creates and manages a connection with the server web socket endpoint.
	 * On start the web-socket endpoint starts listening to and accepting messages.
	 * On stop the end point stops listening and all active connections are closed. A call to open is non blocking.
	 * Messages are forwarded as events to all associated listeners, ie: objects that implement the IWebSocketServer interface.
	 */
	class NAPAPI WebSocketServerEndPoint : public Device
	{
		friend class IWebSocketServer;
		RTTI_ENABLE(Device)
	public:
		// default constructor
		WebSocketServerEndPoint();

		// destructor
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
		bool isOpen() const;

		/**
		 * Stops the end-point from running, all active connections are closed.
		 */
		virtual void stop() override;

		/**
		 * Sends a message to the specified connection
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to the specified connection
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		int mPort = 80;															///< Property: "Port" to open and listen to for messages.
		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

		nap::Signal<const WebSocketConnection&> connectionOpened;
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionClosed;
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionFailed;
		nap::Signal<const WebSocketConnection&, const WebSocketMessage&> messageReceived;

	private:
		std::unique_ptr<wspp::ServerEndPoint> mEndPoint = nullptr;				///< The websocketpp server end-point
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		std::future<void> mServerTask;											///< The background server thread

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

		/**
		 * Called when an http request is made
		 */
		void onHTTP(wspp::ConnectionHandle con);

		/**
		 * Validates the incoming connection
		 */
		bool onValidate(wspp::ConnectionHandle con);

		/**
		 * Called when the server receives a ping message.
		 * Automatically pongs back.
		 */
		bool onPing(wspp::ConnectionHandle con, std::string msg);

		bool mRunning = false;
	};
}
