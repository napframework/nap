/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio.hpp>

namespace nap
{
	// Wrapping often used web-socket library types
	namespace wspp
	{
        using ConfigTLS = websocketpp::config::asio_tls;							    ///< web-socket low level config
		using Config = websocketpp::config::asio;								    ///< web-socket low level config
		using ConnectionHandle = std::weak_ptr<void>;								///< server / client connection
		using MessagePtr = Config::message_type::ptr;								///< internal message format
		using OpCode = websocketpp::frame::opcode::value;							///< web-socket op codes
		using ConnectionHandler = std::function<void(ConnectionHandle)>;			///< connection established / disconnect handle function
		using MessageHandler = std::function<void(ConnectionHandle, MessagePtr)>;	///< message received handle function
		using EndPoint = websocketpp::endpoint<websocketpp::connection<wspp::Config>, wspp::Config>;
		using ServerEndPoint = websocketpp::server<Config>;							///< Web socket server end point
        using ServerEndPointTLS = websocketpp::server<ConfigTLS>;							///< Web socket server end point
		using ClientEndPoint = websocketpp::client<Config>;							///< Web socket client end point
		using ConnectionPtr = EndPoint::connection_ptr;								///< Shared connection pointer
	}
}
