/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
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
}
