/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalutils.h"

namespace nap
{
	bool isPortalEventHeader(const APIEventPtr& event, utility::ErrorState& error)
	{
		if (!error.check(event->getName() == "portal_message", "portal event header name is not portal_message"))
			return false;

		APIArgument* portal_id = event->getArgumentByName("portal_id");
		if (!error.check(portal_id && portal_id->isString(), "portal event header is missing the portal_id string argument"))
			return false;

		APIArgument* message_type = event->getArgumentByName("message_type");
		if (!error.check(message_type && message_type->isString(), "portal event header is missing the message_type string argument"))
			return false;

		return true;
	}
}
