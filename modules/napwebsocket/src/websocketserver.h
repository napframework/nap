#pragma once

// External Includes
#include <nap/device.h>
#include <memory>

// Forward declare websocketpp server type
/*
namespace websocketpp
{
	template<typename T>
	class server;

	namespace config
	{
		struct asio;
	}

	template<typename T>
	class connection;
}
*/

// External Includes
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

namespace nap
{
	using ServerEndpoint = websocketpp::server<websocketpp::config::asio>;

	/**
	 * Allows for receiving and responding to messages over a websocket
	 */
	class NAPAPI WebsocketServer : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~WebsocketServer() override;

		/**
		* Initialize this server
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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

		int mPort = 9002;	///< Property: "Port" port to open

	private:
		std::unique_ptr<ServerEndpoint> mEndpoint = nullptr;		///< Server endpoint

		// Receives incoming messages
		void messageHandler(websocketpp::connection_hdl hdl, ServerEndpoint::message_ptr msg);
	};
}
