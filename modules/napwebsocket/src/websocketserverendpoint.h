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
	// Wrapping often used web-socket library types
	namespace wspp
	{
		using Config			= websocketpp::config::asio;						///< web-socket low level config
		using Connection		= websocketpp::connection_hdl;						///< server / client connection
		using MessagePtr		= Config::message_type::ptr;						///< internal message format
		using OpCode			= websocketpp::frame::opcode::value;				///< web-socket op codes
		using ConnectionHandler = std::function<void(Connection)>;					///< connection established / disconnect handle function
		using MessageHandler	= std::function<void(Connection, MessagePtr)>;		///< message received handle function
		using ServerEndPoint	= websocketpp::server<Config>;						///< server end-point
	}

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

		// No default constructor
		WebSocketServerEndPoint() = delete;

		/**
		 * Construct a server endpoint using the given port and log levels.
		 * Note that all levels equal to and above the given level are logged.
		 * @param port the port to listen to incoming messages
		 * @param logLevel requested library log level
		 * @param accessLevel requested access / client connection log level.
		 * @param logConnection if server / client connection data is logged
		 */
		WebSocketServerEndPoint(int port, uint32 logLevel, uint32 accessLevel);

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
		void send(const std::string& message, wspp::Connection connection, wspp::OpCode opCode);

		/**
		 * Function that is called when this end point receives a new message
		 * @param handler the message handler to install
		 */
		void setMessageHandler(wspp::MessageHandler handler);

	private:
		wspp::ServerEndPoint mEndPoint;			///< The websocketpp server end-point
		uint32 mLogLevel = 0;					///< Library log level
		uint32 mAccessLogLevel = 0;				///< Log client / server connection data
		int mPort = 80;							///< Port to listen for incoming messages
		std::future<void> mServerTask;			///< The background server thread
		bool mOpen = false;						///< If connection is open

		/**
		 * Runs the end point in a background thread until stopped.
		 */
		void run();

		/**
		 * Called when a new connection is made
		 */
		void connectionOpened(wspp::Connection connection);

		/**
		 * Called when a collection is closed
		 */
		void connectionClosed(wspp::Connection connection);

		/**
		 * Called when a failed connection attempt is made
		 */
		void connectionFailed(wspp::Connection connection);
	};
}
