/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/device.h>
#include <thread>
#include <mutex>

// NAP includes
#include <nap/numeric.h>
#include <concurrentqueue.h>
#include <nap/signalslot.h>

// Local includes
#include "udpadapter.h"
#include "udppacket.h"

namespace nap
{
	/**
	 * The UDP Server connects to an endpoint and receives any UDP packets send to the endpoint.
	 * The server will invoke the packetReceived signal when packets are received.
	 * The signal will be fired on the thread this UDPServer is registered to, see UDPThread.
	 */
	class NAPAPI UDPServer final : public UDPAdapter
	{
		RTTI_ENABLE(UDPAdapter)
	public:
        /**
         * Constructor
         */
        UDPServer();

        /**
         * Destructor
         */
        virtual ~UDPServer();

        void registerListenerSlot(Slot<const UDPPacket&>& slot);

        void removeListenerSlot(Slot<const UDPPacket&>& slot);

		int mPort 						= 13251;		///< Property: 'Port' the port the server socket binds to
		std::string mIPAddress			= "";	        ///< Property: 'IP Address' local ip address to bind to, if left empty will bind to any local address

        std::vector<std::string> mMulticastGroups;      ///< Property: 'Multicast Groups' multicast groups to join
	protected:
        /**
         * packet received signal will be dispatched on the thread this UDPServer is registered to, see UDPThread
         */
        Signal<const UDPPacket&> packetReceived;

        /**
         * Called when server socket needs to be created
         * @param errorState The error state
         * @return: true on success
         */
        virtual bool onStart(utility::ErrorState& errorState) override final;

        /**
         * Called when socket needs to be closed
         */
        virtual void onStop() override final;

		/**
		 * The process function
		 */
		void onProcess() override final;
	private:
		// Server specific ASIO implementation
		class Impl;
        std::unique_ptr<Impl> mImpl;

        // mutex
        std::mutex mMutex;
	};
}
