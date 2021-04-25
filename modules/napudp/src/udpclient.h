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
#include "udpdevice.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * The UdpClient class can be used send UdpPackets to an endpoint
	 */
	class NAPAPI UdpClient : public UdpDevice
	{
		RTTI_ENABLE(UdpDevice)
	public:
		bool init(utility::ErrorState& errorState) override;

		void onDestroy() override;

		/**
		 * copies packet and queues the copied packet for sending
		 */
		void send(const UdpPacket& packet);

		/**
		 * returns whether socket is closed due to an error, thread safe call
		 * @return true if socked is closed due to an error
		 */
		bool getHasError() const{ return mHasError.load(); }

		/**
		 * returns const ref to error string, thread safe if called after getHasError() has returned true
		 * @return const ref to error
		 */
		const std::string& getError() const{ return mError; }
	public:
		int mPort 							= 13251; 		///< Property: 'Port' the port the client socket binds to
		std::string mRemoteIp 				= "10.8.0.3";	///< Property: 'Endpoint' the ip adress the client socket binds to
		bool mThrowOnInitError 				= true;			///< Property: 'ThrowOnFailure' when client fails to bind socket, return false on start
		int  mMaxPacketQueueSize			= 1000;			///< Property: 'MaxQueueSize' maximum of queued packets before throwing an error
		bool mStopOnMaxQueueSizeExceeded 	= true;			///< Property: 'StopOnMaxQueueSizeExceeded' close socket and dispatch error when max queued packets exceeded
	protected:
		/**
		 * The threaded send function
		 */
		void process() override;
	private:

		// ASIO
		asio::io_service 			mIOService;
		asio::ip::udp::socket 		mSocket{mIOService};
		std::vector<nap::uint8>		mBuffer;
		asio::ip::udp::endpoint 	mRemoteEndpoint;

		// Threading
		std::atomic_bool 						mRun;
		moodycamel::ConcurrentQueue<UdpPacket> 	mQueue;

		// Error handling
		std::atomic_bool						mHasError 	= { false };
		std::string 							mError 		= "";
	};
}
