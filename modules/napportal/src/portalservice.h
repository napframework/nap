/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalevent.h"

// External Includes
#include <nap/service.h>
#include <mutex>

namespace nap
{
	// Forward Declares
	class PortalComponentInstance;
	class PortalWebSocketComponentInstance;

	/**
	 * The portal service receives portal events from portal WebSocket components.
	 * It then forwards the incoming events to portal components, based on the message type and portal ID.
	 */
	class NAPAPI PortalService : public Service
	{
		friend class PortalComponentInstance;
		friend class PortalWebSocketComponentInstance;
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
		 * Receives portal events from portal WebSocket components and forwards them to portal components.
		 * @param event the portal event that is to be forwarded
		 * @param wsComponent the portal websocket component sending the event
		 * @param error contains information in case forwarding fails
		 * @return whether forwarding the event succeeded
		 */
		bool sendEvent(PortalEventPtr event, const PortalWebSocketComponentInstance& wsComponent, utility::ErrorState& error);

		/**
		 * Called by the portal component in order to register itself with the service.
		 * @param component the component that wants to register itself.
		 */
		void registerComponent(PortalComponentInstance& component);

		/**
		 * Called by the portal component to de-register itself with the service.
		 * @param component the component to de-register.
		 */
		void removeComponent(PortalComponentInstance& component);

		// All the portal components currently available to the system
		std::vector<PortalComponentInstance*> mComponents;

		// Mutex associated with portal component registration and iteration
		std::mutex mComponentMutex;
	};
}
