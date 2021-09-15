/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <cstdint>
#include <nap/numeric.h>
#include <asio/io_service.hpp>
#include <asio/error_code.hpp>
#include <asio/ip/udp.hpp>


namespace nap
{
	namespace artnet
	{
		inline constexpr const char* protocolID = "Art-Net";	///< Protocol identifier
		inline constexpr uint16 opCode = 0x5000;				///< Packet type identifier
		inline constexpr uint16 minProtVer = 14;				///< Min allowed Art-Net protocol version
		inline constexpr uint16	dataLength = 512;				///< Length of Art-Net buffer in bytes
		inline constexpr uint16	headerLength = 18;				///< Length of Art-Net header in bytes
	}

	class ArtNetReceiver;

	/**
	 * Listens for incoming packets and parses them to ArtDmx packet events
	 */
	class ArtNetListener final
	{
	public:
		ArtNetListener(ArtNetReceiver& receiver, asio::io_service& ioService, const std::string& ip, uint16_t port);

	private:
		void startReceive();
		void handleReceive(const asio::error_code& error, std::size_t size);

		// Buffer size is known at compile time
		using ArtNetBuffer = std::array<uint8, artnet::headerLength + artnet::dataLength>;

		ArtNetReceiver&	mReceiver;			// The Art-Net Receiver that holds the message queue
		asio::ip::udp::socket mSocket;		// The UDP socket that is used to receive data
		ArtNetBuffer mBuffer;				// The buffer used for receiving ArtDmx packets
	};
}
