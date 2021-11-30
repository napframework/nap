/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/core.h>
#include <apievent.h>

namespace nap
{
	namespace portal
	{
		inline constexpr const char* eventHeaderName = "portal_event_header";			///< Name of the portal event header
		inline constexpr const char* portalIDArgName = "portal_id";						///< Name of the argument containing the portal ID in the portal event header
		inline constexpr const char* eventTypeArgName = "portal_event_type";			///< Name of the argument containing the portal event type in the portal event header
		inline constexpr const char* eventTypeRequest = "EPortalEventType::Request";	///< Value of the portal event type argument that maps to EPortalEventType::Request
		inline constexpr const char* eventTypeResponse = "EPortalEventType::Response";	///< Value of the portal event type argument that maps to EPortalEventType::Response
		inline constexpr const char* eventTypeUpdate = "EPortalEventType::Update";		///< Value of the portal event type argument that maps to EPortalEventType::Update
	}

	/**
	 * Enum that describes the type of portal event, which determines the effect of the event
	 */
	enum class EPortalEventType : int
	{
		Request		= 0,	///< Request a descriptor of all the portal items in a portal (from client to server)
		Response	= 1,	///< Respond with a descriptor of all the portal items in a portal (from server to client)
		Update		= 2,	///< Update current values of portal items in a portal (bi-directional between client and server)
		Invalid		= -1	///< Not recognized as a valid portal event type
	};

	/**
	 * Checks whether the given API event passes as a valid portal event header
	 * @param event the API event to validate as a valid portal event header
	 * @param error contains error information when the event is not valid
	 * @return whether the event passes as a valid portal event header
	 */
	bool isPortalEventHeader(const APIEventPtr& event, utility::ErrorState& error);

	/**
	 * Retrieves the portal ID from a valid portal event header
	 * @param event the API event to extract the portal ID from
	 * @return the extracted portal ID
	 */
	std::string getPortalID(const APIEventPtr& event);

	/**
	 * Retrieves the portal event type from a valid portal event header
	 * @param event the API event to extract the portal event type from
	 * @return the extracted portal event type
	 */
	EPortalEventType getPortalEventType(const APIEventPtr& event);
}
