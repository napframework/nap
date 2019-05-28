#pragma once

// External Includes
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <memory.h>
#include <websocketutils.h>
#include <future>
#include <utility/errorstate.h>

namespace nap
{
	/**
	 * Server endpoint role.
	 * Creates and manages a connection with the server web socket endpoint.
	 * Call 'open' to start listening and accepting messages.
	 * Call 'close' to close all active connections and stop listening. A call to open is non blocking.
	 * Install handlers to receive messages at run-time. Note that the messages are received on a different thread!
	 * This is a run-time only class that is used internally by other nap resources.
	 */
	class WebSocketServerEndPoint final
	{
	public:
		using PPServerEndPoint = websocketpp::server<websocketpp::config::asio>;

		// No default constructor
		WebSocketServerEndPoint() = delete;

		/**
		 * Construct a server endpoint using the given port and log level
		 * Note that all log levels equal to and above the given level are logged
		 * @param port the port to listen to incoming messages
		 * @param logLevel requested library log level
		 * @param logConnection if server / client connection data is logged
		 */
		WebSocketServerEndPoint(int port, uint32 logLevel, bool logConnectionAccess);

		/**
		 * Stops the end point from running
		 */
		~WebSocketServerEndPoint();

		/**
		 * Opens the port and starts the run loop.
		 * @param error contains the error if opening failed
		 * @return if the port could be opened
		 */
		bool open(nap::utility::ErrorState& error);

		/**
		 * @return if the current end point is open and running
		 */
		bool isOpen() const													{ return mOpen; }

		/**
		 * Stops the end-point from running, all active connections are closed.
		 */
		void close();

		/**
		 * Sends a message to the incoming connection
		 * @param message the message to send
		 * @param connection client to send message to
		 * @param message the original client message
		 */
		void send(const std::string& message, websocketpp::connection_hdl connection, PPServerEndPoint::message_ptr originalMessage);

		/**
		 * Function that is called when the this end point receives a new message
		 * @param handler the message handler to install
		 */
		void setMessageHandler(std::function<void(websocketpp::connection_hdl, PPServerEndPoint::message_ptr)> handler);

	private:
		PPServerEndPoint mEndPoint;				///< The websocketpp server end-point
		uint32 mLogLevel = 0;					///< Library log level
		int mPort = 80;							///< Port to listen for incoming messages
		bool mLogConnectionAccess = true;		///< Log client / server connection data
		std::future<void> mServerTask;			///< The background server thread
		bool mOpen = false;						///< If connection is open

		void run();
	};
}
