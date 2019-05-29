#pragma once

// Local Includes
#include "websocketserverendpoint.h"

// External Includes
#include <nap/device.h>
#include <memory>

// External Includes
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

namespace nap
{
	class WebSocketServerEndPoint;

	/**
	 * Allows for receiving and responding to messages over a web socket.
	 * On start the server end point is opened and ready to accept client messages.
	 * On stop all active connections are closed and the end point stops listening.
	 */
	class NAPAPI WebsocketServer : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~WebsocketServer() override;

		/**
		 * Starts the server
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the server and therefore server end point. All connections are closed.
		 */
		virtual void stop() override;

		int mPort = 80;														///< Property: "Port" to open and listen to for messages.
		bool mLogConnectionUpdates = true;									///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;	///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	private:
		std::unique_ptr<WebSocketServerEndPoint> mEndpoint = nullptr;		///< Server endpoint

		// Receives incoming messages
		void messageHandler(wspp::Connection con, wspp::MessagePtr msg);
	};
}
