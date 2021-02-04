/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

//
#include <nap/resource.h>
#include <nap/resourceptr.h>

// Internal includes
#include "udppacket.h"
#include "udpserver.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class UdpServer;

	/**
	 * UdpServer listeners registers itself to an UdpServer on init and removes itself from server on onDestroy
	 * The UdpServer is a base class that can be extended, the virtual method onUdpPacket needs to be implemented
	 */
	class NAPAPI UdpServerListener : public Resource
	{
		friend class UdpServer;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * upon init, the listener registers itself to the server
		 * @param errorState contains any errors
		 * @return true on successful initialization
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * upon destruction, removes itself from the server
		 */
		virtual void onDestroy() override;

		/**
		 * Enqueues a task on the UdpServer thread
		 * @param task the task to be executed on UdpServer thread
		 */
		void enqueueTask(std::function<void()> task){ mServer->enqueueTask(task); }
	public:
		ResourcePtr<UdpServer> mServer = nullptr; ///< Property: 'Server' resource pointer to UdpServer resource
	protected:
		/**
		 * called by the UdpServer on UdpServer thread when packet is received
		 * @param packet const reference to the UdpPacket
		 */
		virtual void onUdpPacket(const UdpPacket& packet) = 0;
	};
}
