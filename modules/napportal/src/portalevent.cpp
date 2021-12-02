/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalevent.h"
#include "portalutils.h"

// External Includes
#include <rtti/rttiutilities.h>
#include <rtti/jsonwriter.h>
#include <apimessage.h>

namespace nap
{
	bool PortalEvent::toAPIMessageJSON(std::string& outJSON, utility::ErrorState& error)
	{
		std::vector<APIMessage> messages;
		nap::rtti::ObjectList objects;

		// Add portal event header
		APIEventPtr header_event = createPortalEventHeader(mHeader);
		messages.emplace_back(*header_event);
		objects.emplace_back(&messages.back());

		// Add portal item events
		for (APIEvent* event : getAPIEvents())
		{
			messages.emplace_back(*event);
			objects.emplace_back(&messages.back());
		}

		rtti::JSONWriter writer;
		if (!serializeObjects(objects, writer, error))
			return false;

		outJSON = writer.GetJSON();
		return true;
	}


	void PortalEvent::addAPIEvent(APIEventPtr apiEvent)
	{
		mAPIEvents.emplace_back(std::move(apiEvent));
	}


	const APIEvent* PortalEvent::getAPIEvent(int index) const
	{
		assert(index < mAPIEvents.size() && index >= 0);
		return mAPIEvents[index].get();
	}


	APIEvent* PortalEvent::getAPIEvent(int index)
	{
		assert(index < mAPIEvents.size() && index >= 0);
		return mAPIEvents[index].get();
	}
}
