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
		 * Stops the server
		 */
		virtual void stop() override;

		int mPort = 80;					///< Property: "Port" to open and listen to

	private:
		std::unique_ptr<WebSocketServerEndPoint> mEndpoint = nullptr;		///< Server endpoint

		// Receives incoming messages
		void messageHandler(websocketpp::connection_hdl hdl, WebSocketServerEndPoint::PPServerEndPoint::message_ptr msg);
	};
}
