/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitemlayout.h"

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
		inline constexpr const char* itemClampArgName = "portal_item_clamp";			///< Name of the argument containing the clamp value in the portal item message
	    inline constexpr const char* dropDownItemNames = "portal_dropdown_item_names";  ///< Name of the argument containing the dropdown item names in a drop down item message
        inline constexpr const char* itemVisibleArgName = "portal_item_visible";		///< Name of the argument containing the portal item visibility in the portal item message
        inline constexpr const char* itemEnabledArgName = "portal_item_enabled";		///< Name of the argument containing the portal item enabled state in the portal item message
        inline constexpr const char* itemPaddingArgName = "portal_item_padding";        ///< Name of the argument containing the portal item padding in the portal item message
        inline constexpr const char* itemColorArgName = "portal_item_color";            ///< Name of the argument containing the portal item color in the portal item message
        inline constexpr const char* itemFontWeightArgName = "portal_item_font_weight"; ///< Name of the argument containing the portal item font weight in the portal item message
        inline constexpr const char* itemAlignmentArgName = "portal_item_alignment";    ///< Name of the argument containing the portal item alignment in the portal item message
        inline constexpr const char* itemFontSizeArgName = "portal_item_fontsize";      ///< Name of the argument containing the portal item font size in the portal item message
        inline constexpr const char* itemSelectedArgName = "portal_item_selected";      ///< Name of the argument containing the portal item highlight in the portal item message
        inline constexpr const char* itemWidthArgName = "portal_item_width";            ///< Name of the argument containing the portal item width in the portal item message
    }

	/**
	 * Enum that describes the type of portal event, which determines the effect of the event
	 */
	enum class EPortalEventType : int
	{
		Request		    = 0,	///< Request a descriptor of all the portal items in a portal (from client to server)
		Response	    = 1,	///< Respond with a descriptor of all the portal items in a portal (from server to client)
		ValueUpdate		= 2,	///< Update current values of portal items in a portal (bi-directional between client and server)
		StateUpdate     = 3,    ///< Update current state of portal items in a portal
		Reload          = 4,
		Invalid		    = -1	///< Not recognized as a valid portal event type
	};

	/**
	 * Enum that contains the different events that can be triggered from a portal item button
	 */
	enum class EPortalItemButtonEvent : int
	{
		Click = 0,		///< Triggered from a portal item button when it registers a click event
		Press = 1,		///< Triggered from a portal item button when it registers a press event
		Release = 2,	///< Triggered from a portal item button when it registers a release event
		Invalid = -1	///< Not recognized as a valid portal item button event
	};

	/**
	 * Object containing all portal event header information
	 */
	struct NAPAPI PortalEventHeader
	{
		std::string mID;										///< Unique ID of the portal event
		std::string mPortalID;									///< Unique ID of the sending / receiving portal
		EPortalEventType mType = EPortalEventType::Invalid;		///< Type of the portal event, determines the effect
	};

	/**
	 * Converts a portal event type to a string representation
	 * @param type the portal event type to convert to a string
	 * @return A string representing the provided portal event type
	 */
	NAPAPI std::string getPortalEventTypeString(const EPortalEventType& type);

	/**
	 * Converts a string representation to a portal event type
	 * @param type the string to convert to a portal event type
	 * @return The portal event type derived from the string
	 */
	NAPAPI EPortalEventType getPortalEventType(const std::string& type);

	/**
	 * Converts a portal item button event to a string representation
	 * @param event the portal item button event to convert to a string
	 * @return A string representing the provided portal item button event
	 */
	NAPAPI std::string getPortalItemButtonEventString(const EPortalItemButtonEvent& event);

	/**
	 * Converts a string representation to a portal item button event
	 * @param event the string to convert to a portal item button event
	 * @return The portal item button event derived from the string
	 */
	NAPAPI EPortalItemButtonEvent getPortalItemButtonEvent(const std::string& event);

    NAPAPI std::string getPortalItemAlignmentTypeString(const EPortalItemAlignment& alignment);

    NAPAPI void addLayoutArguments(nap::APIEventPtr &event, const PortalItemLayout& layout);

	/**
	 * Attempts to extract a portal event header from a portal event header API event
	 * @param event the API event to extract the portal event header information from
	 * @param outHeader the portal event header to write the information to
	 * @param error contains information when the extraction fails
	 * @return whether the portal event header extraction succeeded
	 */
	NAPAPI bool extractPortalEventHeader(const APIEventPtr& event, PortalEventHeader& outHeader, utility::ErrorState& error);

	/**
	 * Creates an API event that functions as event header in a portal event
	 * @param header the portal event header to be converted to API event
	 * @return an API event that functions as event header in a portal event
	 */
	NAPAPI APIEventPtr createPortalEventHeader(const PortalEventHeader& header);
}
