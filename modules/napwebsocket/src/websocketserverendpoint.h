#pragma once

// External Includes
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <memory.h>
#include <websocketutils.h>
#include <future>

namespace nap
{
	/**
	 * Server endpoint role.
	 * Creates and manages a connection with the server WebSocket endpoint.
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

		~WebSocketServerEndPoint();

		/**
		 * Opens the port and starts the run loop. This is a non blocking call.
		 */
		void open();

		/**
		 * Closes the port, all active connections are closed.
		 */
		void close();

		/**
		 * Sends a message to the incoming connection
		 * @param message the message to send
		 * @param connection client to send message to
		 * @param message the original client message
		 */
		void send(const std::string& message, websocketpp::connection_hdl connection, PPServerEndPoint::message_ptr originalMessage);

		void setHandler(std::function<void(websocketpp::connection_hdl, PPServerEndPoint::message_ptr)> message_handler);

	private:
		PPServerEndPoint mEndPoint;				///< The websocketpp server end-point
		uint32 mLogLevel = 0;					///< Library log level
		int mPort = 80;							///< Port to listen for incoming messages
		bool mLogConnectionAccess = true;		///< Log client / server connection data
		std::future<void> mServerTask;			///< The background server thread

		void run();
	};
}
