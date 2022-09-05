/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetlistener.h"
#include "artnetreceiver.h"
#include "artnetevent.h"

// External includes
#include <utility>
#include <memory>
#include <string>
#include <cstring>
#include <functional>


namespace nap
{

	ArtNetListener::ArtNetListener(ArtNetReceiver& receiver, asio::io_service& ioService, const std::string& ip, uint16 port)
		: mReceiver(receiver)
		, mSocket(ioService, ip.empty()
			? asio::ip::udp::endpoint(asio::ip::udp::v4(), port)
			: asio::ip::udp::endpoint(asio::ip::make_address(ip), port))
	{
		// Start receiving UDP packets
		startReceive();
	}

	void ArtNetListener::startReceive()
	{
		// Start receiving UDP asynchronously
		mSocket.async_receive
		( 
			asio::buffer(mBuffer.data(), mBuffer.size()),
			std::bind(&ArtNetListener::handleReceive, this, std::placeholders::_1, std::placeholders::_2)
		);
	}


	void ArtNetListener::handleReceive(const asio::error_code& error, std::size_t size)
	{
		if (!error)
		{
			// Confirm the ID field for the ArtDmx packet
			std::string protocolId((char*)mBuffer.data(), 7);
			if (protocolId == artnet::protocolID)
			{
				// Check the OpCode and ProtVer fields for the ArtDmx packet
				uint16 opCode = (mBuffer[9] << 8) | (mBuffer[8] & 0xff);
				uint16 protVer = (mBuffer[10] << 8) | (mBuffer[11] & 0xff);
				if (opCode == artnet::opCode && protVer >= artnet::minProtVer)
				{
					// Decode the rest of the Art-Net package
					uint8 sequence = mBuffer[12];
					uint8 physical = mBuffer[13];
					uint16 portAddress = (mBuffer[15] << 8) | (mBuffer[14] & 0xff);
					uint16 dataLength = (mBuffer[16] << 8) | (mBuffer[17] & 0xff);

					// Create out Art-Net event
					ArtNetEventPtr event = std::make_unique<ArtNetEvent>(sequence, physical, portAddress);

					// Set the channel data
					event->setData(mBuffer.data() + artnet::headerLength, dataLength);

					// Add event to receiver
					mReceiver.addEvent(std::move(event));
				}
			}

			// Reset the buffer
			std::fill(mBuffer.begin(), mBuffer.end(), 0);

			// Receive more data
			startReceive();
		}
	}
}
