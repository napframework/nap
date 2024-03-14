/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalutils.h"

// External includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::EPortalEventType)
	RTTI_ENUM_VALUE(nap::EPortalEventType::Request, "Request"),
	RTTI_ENUM_VALUE(nap::EPortalEventType::Response, "Response"),
	RTTI_ENUM_VALUE(nap::EPortalEventType::ValueUpdate, "ValueUpdate"),
    RTTI_ENUM_VALUE(nap::EPortalEventType::StateUpdate, "StateUpdate"),
    RTTI_ENUM_VALUE(nap::EPortalEventType::Reload, "Reload"),
    RTTI_ENUM_VALUE(nap::EPortalEventType::OpenDialog, "OpenDialog"),
    RTTI_ENUM_VALUE(nap::EPortalEventType::DialogClosed, "DialogClosed"),
	RTTI_ENUM_VALUE(nap::EPortalEventType::Invalid, "Invalid")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EPortalItemButtonEvent)
	RTTI_ENUM_VALUE(nap::EPortalItemButtonEvent::Click, "Click"),
	RTTI_ENUM_VALUE(nap::EPortalItemButtonEvent::Press, "Press"),
	RTTI_ENUM_VALUE(nap::EPortalItemButtonEvent::Release, "Release"),
	RTTI_ENUM_VALUE(nap::EPortalItemButtonEvent::Invalid, "Invalid")
RTTI_END_ENUM

namespace nap
{
	std::string getPortalEventTypeString(const EPortalEventType& type)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EPortalEventType);
		return enum_type.get_enumeration().value_to_name(type).to_string();
	}


	EPortalEventType getPortalEventType(const std::string& type)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EPortalEventType);
		rtti::Variant var = enum_type.get_enumeration().name_to_value(type.data());
		return var.is_valid() ? var.get_value<EPortalEventType>() : EPortalEventType::Invalid;
	}


	std::string getPortalItemButtonEventString(const EPortalItemButtonEvent& event)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EPortalItemButtonEvent);
		return enum_type.get_enumeration().value_to_name(event).to_string();
	}


	EPortalItemButtonEvent getPortalItemButtonEvent(const std::string& event)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EPortalItemButtonEvent);
		rtti::Variant var = enum_type.get_enumeration().name_to_value(event.data());
		return var.is_valid() ? var.get_value<EPortalItemButtonEvent>() : EPortalItemButtonEvent::Invalid;
	}

    std::string getPortalItemAlignmentTypeString(const EPortalItemAlignment& alignment)
    {
        rtti::TypeInfo enum_type = RTTI_OF(EPortalItemAlignment);
        return enum_type.get_enumeration().value_to_name(alignment).to_string();
    }


	bool extractPortalID(const APIEventPtr& event, std::string& outID, utility::ErrorState& error)
	{
		// Check the portal id argument
		APIArgument* arg = event->getArgumentByName(portal::portalIDArgName);
		if (!error.check(arg != nullptr && arg->isString(), "portal event header is missing the %s string argument", portal::portalIDArgName))
			return false;

		outID = arg->asString();
		return true;
	}


	bool extractPortalEventType(const APIEventPtr& event, EPortalEventType& outType, utility::ErrorState& error)
	{
		// Check the portal event type argument
		APIArgument* arg = event->getArgumentByName(portal::eventTypeArgName);
		if (!error.check(arg != nullptr && arg->isString(), "portal event header is missing the %s string argument", portal::eventTypeArgName))
			return false;

		// Ensure the portal event type is valid
		std::string typeStr = arg->asString();
		EPortalEventType type = getPortalEventType(typeStr);
		if (!error.check(type != EPortalEventType::Invalid, "not a valid portal event type: %s", typeStr.c_str()))
			return false;

		outType = type;
		return true;
	}


	bool extractPortalEventHeader(const APIEventPtr& event, PortalEventHeader& outHeader, utility::ErrorState& error)
	{
		if (!error.check(event->getName() == portal::eventHeaderName, "portal event header name is not %s", portal::eventHeaderName))
			return false;

		if (!extractPortalID(event, outHeader.mPortalID, error))
			return false;

		if (!extractPortalEventType(event, outHeader.mType, error))
			return false;

		outHeader.mID = event->getID();
		return true;
	}


	APIEventPtr createPortalEventHeader(const PortalEventHeader& header)
	{
		APIEventPtr event = std::make_unique<APIEvent>(nap::portal::eventHeaderName, header.mID);
		event->addArgument<APIString>(nap::portal::portalIDArgName, header.mPortalID);
		event->addArgument<APIString>(nap::portal::eventTypeArgName, getPortalEventTypeString(header.mType));
		return event;
	}

    void addLayoutArguments(nap::APIEventPtr &event, const PortalItemLayout& layout)
    {
        event->addArgument<APIValue<std::vector<float>>>(nap::portal::itemPaddingArgName, layout.mPadding.toVector());
        event->addArgument<APIValue<bool>>(nap::portal::itemVisibleArgName, layout.mVisible);
        event->addArgument<APIValue<bool>>(nap::portal::itemEnabledArgName, layout.mEnabled);
        event->addArgument<APIValue<bool>>(nap::portal::itemSelectedArgName, layout.mSelected);
        event->addArgument<APIValue<float>>(nap::portal::itemWidthArgName, layout.mWidth);
    }
}
