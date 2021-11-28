/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <portalapicomponent.h>
#include <websocketconnection.h>
#include <apimessage.h>

namespace nap
{
	// Forward declares
	class PortalComponentInstance;

	/**
	 * Handles communication between the NAP application and a web portal.
	 */
	class NAPAPI PortalComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PortalComponent, PortalComponentInstance)

	public:

		ComponentPtr<PortalAPIComponent> mPortalAPIComponent;	///< Property: 'PortalAPIComponent' the portal API component that can forward messages to this portal
	};


	/*
	 * Handles communication between the NAP application and a web portal.
	 */
	class NAPAPI PortalComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:

		/**
		 * Constructor
		 */
		PortalComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Initializes the portal component
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Get the portal API component that can forward messages to this portal
		const ComponentInstancePtr<PortalAPIComponent>& getPortalAPIComponent()
		{
			return mPortalAPIComponent;
		};

		// The portal API component that can forward messages to this portal
		ComponentInstancePtr<PortalAPIComponent> mPortalAPIComponent = { this, &PortalComponent::mPortalAPIComponent };

	private:

		/**
		 * Called by the portal service when this component receives a portal request
		 * @param connection the connection that performed the portal request
		 */
		void processPortalRequest(const WebSocketConnection& connection);

		/**
		 * Called by the portal service when this component receives a portal update
		 * @param messages the API messages containing the updated portal item values
		 */
		void processPortalUpdate(const std::vector<APIMessage*>& messages);
	};
}
