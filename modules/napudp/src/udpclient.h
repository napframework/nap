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
#include "udppacket.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * The UdpClient class can be used send UdpPackets to an endpoint
	 */
	class NAPAPI UdpClient : public Device
	{
		RTTI_ENABLE(Device)
	public:
		/**
		 * on start, binds socket to endpoint and fires up a thread responsible for sending the UdpPackets to endpoint
		 * @param errorState contains any error
		 * @return true on successful initialization
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * stops thread and closes socket
		 */
		virtual void stop() override;

		/**
		 * copies packet and queues the copied packet for sending
		 */
		void send(const UdpPacket& packet);
	public:
		int mPort 				= 13251; 		///< Property: 'Port' the port the client socket binds to
		std::string mRemoteIp 	= "10.8.0.3";	///< Property: 'Endpoint' the ip adress the client socket binds to
		bool mThrowOnInitError 	= true;			///< Property: 'ThrowOnFailure' when client fails to bind socket, return false on start
	private:
		/**
		 * The threaded send function
		 */
		void sendThread();

		// ASIO
		asio::io_service 			mIOService;
		asio::ip::udp::socket 		mSocket{mIOService};
		std::vector<nap::int8>		mBuffer;
		asio::ip::udp::endpoint 	mRemoteEndpoint;

		// Threading
		std::thread 							mSendThread;
		std::atomic_bool 						mRun;
		moodycamel::ConcurrentQueue<UdpPacket> 	mQueue;
	};
}
