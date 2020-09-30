/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <ip/UdpSocket.h>
#include <nap/resource.h>
#include <rtti/factory.h>
#include <queue>
#include <nap/device.h>

// Local Includes
#include "oscevent.h"

// Forward declares
class UdpTransmitSocket;
namespace nap
{
	class OSCService;
}
namespace osc
{
	class OutboundPacketStream;
}


namespace nap 
{
	/**
	 * Sends a single OSC message or bundle of OSC messages.
	 * This device manages it's own connection and is constructed using a target ip address and port.
	 */
	class NAPAPI OSCSender : public Device
	{
		friend class OSCService;
		RTTI_ENABLE(Device)
	public:
		OSCSender() = default;

		// Constructor used by factory
		OSCSender(OSCService& service);

		std::string mIPAddress = "127.0.0.1";	///< Property: 'IpAddress' target machine ip address
		int mPort = 8000;			            ///< Property: 'Port' target machine port
		int mBufferScale = 2;					///< Property: 'Scale' scale factor applied to OSC message buffer.


		/**
		 * Constructs the host end point and communication socket.
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 *	Deletes the communication socket
		 */
		virtual void stop() override;

		/**
		 * Sends an OSC message immediately without adding it to the queue
		 * Note that it's more efficient to add a message to the queue and send them as a bundle
		 * @param oscEvent the event to send
		 */
		bool send(const OSCEvent& oscEvent);

		/**
		 * Adds an event to the queue
		 * This is more efficient than sending over an individual message
		 * Call send() to send over the queue as one message bundle
		 * @param oscEvent the event to add to the queue, note that the sender
		 * takes ownership of the event
		 */
		void addEvent(OSCEventPtr oscEvent);

		/**
		 * Sends all the osc events in the queue as a bundle
		 * All events that were send are removed from the queue
		 */
		void sendQueuedEvents();

	private:
		// Socket used for sending messages
		std::unique_ptr<UdpTransmitSocket> mSocket = nullptr;

		/**
		 * Adds an oscEvent to the package this is send
		 * @param outPacket the packet to populate
		 */
		void writeToPacket(const OSCEvent& oscEvent, osc::OutboundPacketStream& outPacket);

		// Queue that holds all the events to be send over as a bundle
		std::queue<OSCEventPtr> mEventQueue;
		
		// The number of bytes represented by the values in the event queue
		std::size_t	mEventQueueDataSize;

		// Vector that holds osc data to be send over
		std::vector<char> mBuffer;
	};
}
