#pragma once

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

namespace nap
{
	// Wrapping often used web-socket library types
	namespace wspp
	{
		using Config = websocketpp::config::asio;									///< web-socket low level config
		using ConnectionHandle = std::weak_ptr<void>;								///< server / client connection
		using MessagePtr = Config::message_type::ptr;								///< internal message format
		using OpCode = websocketpp::frame::opcode::value;							///< web-socket op codes
		using ConnectionHandler = std::function<void(ConnectionHandle)>;			///< connection established / disconnect handle function
		using MessageHandler = std::function<void(ConnectionHandle, MessagePtr)>;	///< message received handle function
		using EndPoint = websocketpp::endpoint<websocketpp::connection<wspp::Config>, wspp::Config>;
		using ServerEndPoint = websocketpp::server<Config>;							///< Web socket server end point specialization
		using ConnectionPtr = EndPoint::connection_ptr;								///< Shared connection pointer
	}
}
