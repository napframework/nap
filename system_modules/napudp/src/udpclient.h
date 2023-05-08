/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/device.h>
#include <queue>
#include <mutex>

// NAP includes
#include <utility/threading.h>
#include <concurrentqueue.h>

// Local includes
#include "udpadapter.h"
#include "udppacket.h"

namespace nap
{
	/**
	 * The UDP Client class is used to send UDP Packets to an endpoint.
	 */
	class NAPAPI UDPClient final : public UDPAdapter
	{
		RTTI_ENABLE(UDPAdapter)
	public:
        /**
         * Constructor
         */
        UDPClient();

        /**
         * Destructor
         */
        virtual ~UDPClient();

		/**
		 * Makes a copy of the packet and queues if for sending.
		 * Call send(std::move(packet)) if you want to move the packet instead.
		 * Calling this method is Thread-Safe.
		 * @param packet to send
		 */
		void send(const UDPPacket& packet);

		/**
		 * Moves and queues the packet for sending.
		 * Calling this method is Thread-Safe.
		 * @param packet to send
		 */
		void send(UDPPacket&& packet);

		int mPort 							= 13251; 		///< Property: 'Port' the port the client socket binds to
		std::string mEndpoint 				= "10.8.0.3";	///< Property: 'Endpoint' the ip address the client socket binds to
		int  mMaxPacketQueueSize			= 1000;			///< Property: 'MaxQueueSize' maximum of queued packets
		bool mStopOnMaxQueueSizeExceeded 	= true;			///< Property: 'StopOnMaxQueueSizeExceeded' stop adding packets when queue size is exceed
		bool mBroadcast                     = false;        ///< Property: 'Broadcast' set option to broadcast

	protected:
        /**
         * Starts the UDP client and creates the socket
         * @param error contains error information
         * @return true on success
         */
        bool onStart(utility::ErrorState& errorState) override final;

        /**
         * Called when socket needs to be closed
         */
        void onStop() override final;

		/**
		 * The process function
		 */
		void onProcess() override final;
	private:
		// Client specific ASIO implementation
		class Impl;
        std::unique_ptr<Impl> mImpl;
        std::vector<nap::uint8> mBuffer;

		// Threading
		moodycamel::ConcurrentQueue<UDPPacket> 	mQueue;
	};
}
