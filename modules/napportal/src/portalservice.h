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
	 * The portal service
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
