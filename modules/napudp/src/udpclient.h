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
	 * The UDPClient class can be used send UdpPackets to an endpoint
	 */
	class NAPAPI UDPClient final : public UDPAdapter
	{
		RTTI_ENABLE(UDPAdapter)
	public:
		/**
		 * initialization
		 * @param error contains error information
		 * @return true on succes
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * called on destruction
		 */
		void onDestroy() override;

		/**
		 * copies packet and queues the copied packet for sending
		 */
		void copyQueuePacket(const UDPPacket& packet);

		/**
		 * moves packet and queues the moved packet for sending
		 * @param packet to move
		 */
		void moveQueuePacket(UDPPacket& packet);
	public:
		// properties
		int mPort 							= 13251; 		///< Property: 'Port' the port the client socket binds to
		std::string mRemoteIp 				= "10.8.0.3";	///< Property: 'Endpoint' the ip adress the client socket binds to
		bool mThrowOnInitError 				= true;			///< Property: 'ThrowOnFailure' when client fails to bind socket, return false on start
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
