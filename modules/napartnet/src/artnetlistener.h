/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <cstdint>
#include <asio/io_service.hpp>
#include <asio/error_code.hpp>
#include <asio/ip/udp.hpp>


namespace nap
{
	class ArtNetReceiver;

	/**
	* Listens for incoming packages and parses them to ArtDmx packet events
	*/
	class ArtNetListener
	{

	public:

		ArtNetListener(ArtNetReceiver& receiver, asio::io_service& ioService, uint16_t port);
		~ArtNetListener();

	private:

		void startReceive();
		void handleReceive(const asio::error_code& error, std::size_t size);

		static constexpr const char*	PROTOCOL_ID = "Art-Net";
		static constexpr uint16_t		OP_CODE = 0x5000;
		static constexpr uint16_t		MIN_PROT_VER = 14;
		static constexpr uint16_t		DATA_LENGTH = 512;
		static constexpr uint8_t		HEADER_LENGTH = 18;

		ArtNetReceiver&					mReceiver;		// The Art-Net Receiver that holds the message queue
		asio::ip::udp::socket			mSocket;		// The UDP socket that is used to receive data
		size_t							mBufferSize;	// The maximum size for the ArtDmx package in bytes
		uint8_t*						mBuffer;		// The buffer used for receiving ArtDmx packages
	};
}
