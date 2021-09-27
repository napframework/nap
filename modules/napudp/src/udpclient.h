/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/device.h>
#include <queue>
#include <mutex>

// ASIO includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>

// NAP includes
#include <utility/threading.h>
#include <concurrentqueue.h>

// Local includes
#include "udpadapter.h"
#include "udppacket.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * The UDP Client class is used to send UDP Packets to an endpoint.
	 */
	class NAPAPI UDPClient final : public UDPAdapter
	{
		RTTI_ENABLE(UDPAdapter)
	public:
		/**
		 * Initializes the UDP client
		 * @param error contains error information
		 * @return true on success
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * called on destruction
		 */
		void onDestroy() override;

		/**
		 * Makes a copy of the packet and queues if for sending.
		 * Call send(std::move(packet)) if you want to move the packet instead.
		 * @param packet the packet to copy and queue
		 */
		void send(const UDPPacket& packet);

		/**
		 * Moves and queues the packet for sending.
		 * @param packet to move and queue
		 */
		void send(UDPPacket&& packet);

	public:
		// properties
		int mPort 							= 13251; 		///< Property: 'Port' the port the client socket binds to
		std::string mRemoteIp 				= "10.8.0.3";	///< Property: 'Endpoint' the ip address the client socket binds to
		int  mMaxPacketQueueSize			= 1000;			///< Property: 'MaxQueueSize' maximum of queued packets
		bool mStopOnMaxQueueSizeExceeded 	= true;			///< Property: 'StopOnMaxQueueSizeExceeded' stop adding packets when queue size is exceed
	protected:
		/**
		 * The process function
		 */
		void process() override;
	private:
		// ASIO
		asio::io_service 			mIOService;
		asio::ip::udp::socket 		mSocket{mIOService};
		std::vector<nap::uint8>		mBuffer;
		asio::ip::udp::endpoint 	mRemoteEndpoint;

		// Threading
		moodycamel::ConcurrentQueue<UDPPacket> 	mQueue;
	};
}
