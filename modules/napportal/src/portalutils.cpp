/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalutils.h"

namespace nap
{
	std::string getPortalEventTypeString(const EPortalEventType& type)
	{
		switch (type)
		{
		case EPortalEventType::Request:
			return nap::portal::eventTypeRequest;

		case EPortalEventType::Response:
			return nap::portal::eventTypeResponse;

		case EPortalEventType::Update:
			return nap::portal::eventTypeUpdate;

		default:
			return nap::portal::eventTypeInvalid;
		}
	}


	EPortalEventType getPortalEventType(const std::string& type)
	{
		if (type == portal::eventTypeRequest)
			return EPortalEventType::Request;

		if (type == portal::eventTypeResponse)
			return EPortalEventType::Response;

		if (type == portal::eventTypeUpdate)
			return EPortalEventType::Update;

		return EPortalEventType::Invalid;
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
}
