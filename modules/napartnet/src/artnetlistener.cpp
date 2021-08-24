/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "artnetlistener.h"
#include "artnetreceiver.h"
#include "artdmxpacketevent.h"

// External includes
#include <utility>
#include <memory>
#include <string>
#include <cstring>
#include <functional>
#include <asio.hpp>


namespace nap
{

	static asio::io_service* toService(void* ioService)
	{
		return reinterpret_cast<asio::io_service*>(ioService);
	}


	static asio::ip::udp::socket* toSocket(void* socket)
	{
		return reinterpret_cast<asio::ip::udp::socket*>(socket);
	}


	static void handleReceive(ArtNetListener* self, const asio::error_code& error, std::size_t size)
	{
		if (!error)
		{
			// Confirm the ID field for the ArtDmx packet
			std::string protocolId((char*)self->mBuffer, 7);
			if (protocolId == self->PROTOCOL_ID)
			{
				// Check the OpCode and ProtVer fields for the ArtDmx packet
				uint16_t opCode = (self->mBuffer[9] << 8) | (self->mBuffer[8] & 0xff);
				uint16_t protVer = (self->mBuffer[10] << 8) | (self->mBuffer[11] & 0xff);
				if (opCode == self->OP_CODE && protVer >= self->MIN_PROT_VER)
				{
					// Decode the rest of the Art-Net package
					uint8_t sequence = self->mBuffer[12];
					uint8_t physical = self->mBuffer[13];
					uint16_t portAddress = (self->mBuffer[15] << 8) | (self->mBuffer[14] & 0xff);
					uint16_t dataLength = (self->mBuffer[16] << 8) | (self->mBuffer[17] & 0xff);

					// Create out Art-Net event
					ArtDmxPacketEventPtr event = std::make_unique<ArtDmxPacketEvent>(sequence, physical, portAddress);

					// Set the channel data
					event->setData(self->mBuffer + self->HEADER_LENGTH, dataLength);

					// Add event to receiver
					self->mReceiver.addEvent(std::move(event));
				}
			}

			// Reset the buffer
			std::memset(self->mBuffer, 0, self->mBufferSize);

			// Receive more data
			self->startReceive();
		}
	}


	ArtNetListener::ArtNetListener(ArtNetReceiver& receiver, void* ioService, uint16_t port)
		: mReceiver(receiver)
		, mSocketHandle(nullptr)
		, mBufferSize(HEADER_LENGTH + DATA_LENGTH)
		, mBuffer(new uint8_t[mBufferSize])
	{
		// Open the ASIO UDP socket
		asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), port);
		mSocketHandle = new asio::ip::udp::socket(*toService(ioService), endpoint);

		// Ensure the buffer starts empty
		std::memset(mBuffer, 0, mBufferSize);

		// Start receiving UDP packages
		startReceive();
	}


	ArtNetListener::~ArtNetListener()
	{
		// Close the ASIO UDP socket
		if (mSocketHandle != nullptr)
			delete toSocket(mSocketHandle);

		// Deallocate our buffer
		delete mBuffer;
	}


	void ArtNetListener::startReceive()
	{
		// Start receiving UDP asynchronously
		toSocket(mSocketHandle)->async_receive(
			asio::buffer(mBuffer, mBufferSize),
			std::bind(&handleReceive, this,
				std::placeholders::_1, std::placeholders::_2));
	}
}
