/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalutils.h"

namespace nap
{
	bool isPortalEventHeader(const APIEventPtr& event, utility::ErrorState& error)
	{
		bool name_valid = event->getName() == portal::eventHeaderName;
		if (!error.check(name_valid, "portal event header name is not %s", portal::eventHeaderName))
			return false;

		APIArgument* id_arg = event->getArgumentByName(portal::portalIDArgName);
		APIArgument* type_arg = event->getArgumentByName(portal::eventTypeArgName);

		bool id_valid = id_arg != nullptr && id_arg->isString();
		bool type_valid = type_arg != nullptr && type_arg->isString();

		if (!error.check(id_valid, "portal event header is missing the %s string argument", portal::portalIDArgName))
			return false;

		if (!error.check(type_valid, "portal event header is missing the %s string argument", portal::eventTypeArgName))
			return false;

		return true;
	}


	std::string getPortalID(const APIEventPtr& event)
	{
		APIArgument* arg = event->getArgumentByName(portal::portalIDArgName);
		assert(arg != nullptr && arg->isString());
		return arg->asString();
	}


	EPortalEventType getPortalEventType(const APIEventPtr& event)
	{
		APIArgument* arg = event->getArgumentByName(portal::eventTypeArgName);
		assert(arg != nullptr && arg->isString());
		std::string type = arg->asString();

		if (type == portal::eventTypeRequest)
			return EPortalEventType::Request;

		if (type == portal::eventTypeResponse)
			return EPortalEventType::Response;

		if (type == portal::eventTypeUpdate)
			return EPortalEventType::Update;

		return EPortalEventType::Invalid;
	}


	void setPortalEventHeader(const APIEventPtr& event, PortalEventHeader& outHeader)
	{
		outHeader.mID = event->getID();
		outHeader.mPortalID = getPortalID(event);
		outHeader.mType = getPortalEventType(event);
	}
}
