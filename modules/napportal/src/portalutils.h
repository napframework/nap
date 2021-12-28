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
		inline constexpr const char* itemTypeArgName = "portal_item_type";				///< Name of the argument containing the portal item type in the portal item message
		inline constexpr const char* itemValueArgName = "portal_item_value";			///< Name of the argument containing the portal item value in the portal item message
		inline constexpr const char* itemMinArgName = "portal_item_min";				///< Name of the argument containing the minimum portal item value in the portal item message
		inline constexpr const char* itemMaxArgName = "portal_item_max";				///< Name of the argument containing the maximum portal item value in the portal item message
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
	 * Converts a portal event type to a string representation
	 * @param type the portal event type to convert to a string
	 * @return A string representing the provided portal event type
	 */
	std::string getPortalEventTypeString(const EPortalEventType& type);

	/**
	 * Attempts to extract a portal event header from a portal event header API event
	 * @param event the API event to extract the portal event header information from
	 * @param outHeader the portal event header to write the information to
	 * @param error contains information when the extraction fails
	 * @return whether the portal event header extraction succeeded
	 */
	bool extractPortalEventHeader(const APIEventPtr& event, PortalEventHeader& outHeader, utility::ErrorState& error);

	/**
	 * Creates an API event that functions as event header in a portal event
	 * @param header the portal event header to be converted to API event
	 * @return an API event that functions as event header in a portal event
	 */
	APIEventPtr createPortalEventHeader(const PortalEventHeader& header);
}
