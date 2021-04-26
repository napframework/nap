/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declares
	class UDPThread;

	/**
	 * The UDPService is responsible for processing any UDPThreads that have registered themselves to receive an
	 * update call by the service. The Update Method of the UDPThread is set to "Main Thread" in that case
	 */
	class NAPAPI UDPService : public Service
	{
		friend class UDPThread;

		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		UDPService(ServiceConfiguration* configuration);

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initialization
		 * @param error contains error information
		 * @return true on succes
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * shuts down the service
		 */
		virtual void shutdown() override;

		/**
		 * update call wil call process on any registered UDPThreads
		 * @param deltaTime time since last udpate
		 */
		virtual void update(double deltaTime) override;
	private:
		/**
		 * registers an UDPThread
		 * @param thread the thread to register
		 */
		void registerUdpThread(UDPThread* thread);

		/**
		 * removes an UDPThread
		 * @param thread the thread do remove
		 */
		void removeUdpThread(UDPThread* thread);
	private:
		// registered udp threads
		std::vector<UDPThread*> mThreads;
	};
}
