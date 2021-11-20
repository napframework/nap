/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <websocketcomponent.h>

namespace nap
{
	// Forward declares
	class PortalInstance;

	/**
	 * Controls all input / output from the NAP application to a web portal.
	 */
	class NAPAPI Portal : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(Portal, PortalInstance)

	public:

		ComponentPtr<WebSocketComponent> mWebSocketComponent;		///< Property: 'WebSocketComponent' the WebSocketComponent to listen to for incoming messages.
	};


	/*
	 * Controls all input / output from the NAP application to a web portal.
	 */
	class NAPAPI PortalInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:

		/**
		 * Constructor
		 */
		PortalInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Destructor
		 */
		virtual ~PortalInstance() override;

		/**
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		ComponentInstancePtr<WebSocketComponent> mWebSocketComponent = { this, &Portal::mWebSocketComponent };

	private:

	};
}
