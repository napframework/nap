/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <websocketservice.h>
#include <mutex>

namespace nap
{
	// Forward Declares
	class PortalWebSocketServer;
	class PortalComponentInstance;

	/**
	 * The portal service
	 */
	class NAPAPI PortalService : public Service
	{
		friend class PortalWebSocketServer;
		friend class PortalComponentInstance;
		RTTI_ENABLE(Service)

	public:

		/**
		 * Default constructor
		 */
		PortalService(ServiceConfiguration* configuration);

		/**
		 * @return the WebSocket service
		 */
		WebSocketService& getWebSocketService();

		/**
		 * @return const ref to the WebSocket service
		 */
		const WebSocketService& getWebSocketService() const;

	protected:
		/**
		 * Registers all objects that need a specific way of construction.
		 * @param factory the factory to register the object creators with.
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * This service depends on the WebSocket service
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Called after creation
		 */
		virtual void created() override;

	private:

		/**
		 * Called by the portal WebSocket server in order to register itself with the service.
		 * @param server the server that wants to register itself.
		 */
		void registerServer(PortalWebSocketServer& server);

		/**
		 * Called by the portal WebSocket server to de-register itself with the service.
		 * @param server the server to de-register.
		 */
		void removeServer(PortalWebSocketServer& server);

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

		// Handle to the WebSocket service
		WebSocketService* mWebSocketService = nullptr;

		// All the portal WebSocket servers currently available to the system
		std::vector<PortalWebSocketServer*> mServers;

		// All the portal components currently available to the system
		std::unordered_map<std::string, PortalComponentInstance*> mComponents;
	};
}
