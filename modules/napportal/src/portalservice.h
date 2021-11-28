/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <mutex>

namespace nap
{
	// Forward Declares
	class PortalComponentInstance;

	/**
	 * The portal service receives API WebSocket events from portal API components.
	 * It then relays the incoming events to portal components, based on the message type and portal ID.
	 */
	class NAPAPI PortalService : public Service
	{
		friend class PortalComponentInstance;
		RTTI_ENABLE(Service)

	public:

		/**
		 * Default constructor
		 */
		PortalService(ServiceConfiguration* configuration);

		/**
		 * Destructor	
		 */
		~PortalService() override;

	private:

		/**
		 * Called by the portal component in order to register itself with the service.
		 * @param portalComponent the component that wants to register itself.
		 */
		void registerPortalComponent(PortalComponentInstance& portalComponent);

		/**
		 * Called by the portal component to de-register itself with the service.
		 * @param portalComponent the component to de-register.
		 */
		void removePortalComponent(PortalComponentInstance& portalComponent);

		// All the portal components currently available to the system
		std::vector<PortalComponentInstance*> mPortalComponents;

		// Mutex associated with portal component registration and iteration
		std::mutex mPortalComponentMutex;
	};
}
