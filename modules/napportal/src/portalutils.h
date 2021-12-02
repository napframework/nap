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
		inline constexpr const char* eventTypeInvalid = "EPortalEventType::Invalid";	///< Value of the portal event type argument that maps to EPortalEventType::Invalid
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
	 * Object containing all portal event header information
	 */
	struct PortalEventHeader
	{
		std::string mID;		///< Unique ID of the portal event
		std::string mPortalID;	///< Unique ID of the sending / receiving portal
		EPortalEventType mType;	///< Type of the portal event, determines the effect
	};

	/**
	 * Attempts to extract a portal event header from a portal event header API event
	 * @param event the API event to extract the portal event header information from
	 * @param outHeader the portal event header to write the information to
	 * @param error contains information when the extraction fails
	 * @return whether the portal event header extraction succeeded
	 */
	bool extractPortalEventHeader(const APIEventPtr& event, PortalEventHeader& outHeader, utility::ErrorState& error);
}
