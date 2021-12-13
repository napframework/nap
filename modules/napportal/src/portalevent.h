/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalutils.h"

// External Includes
#include <nap/core.h>
#include <nap/event.h>
#include <utility/uniqueptrvectoriterator.h>
#include <apievent.h>
#include <websocketconnection.h>

namespace nap
{
	/**
	 * Used for communication between the client and server side of NAP portals.
	 * Contains all the information for the app to decide where to route the event and how to handle it.
	 * EPortalEventType::Request events are sent by clients to request a full description of the portal.
	 * EPortalEventType::Response events are then sent by the server as a response to request events.
	 * EPortalEventType::Update events can be sent both ways and indicate that portal items should be updated.
	 */
	class NAPAPI PortalEvent : public Event
	{
		RTTI_ENABLE(Event)

	public:

		using APIEventList = std::vector<APIEventPtr>;
		using APIEventConstIterator = utility::UniquePtrConstVectorWrapper<APIEventList, APIEvent*>;

		/**
		 * Default constructor
		 * @param header the portal event header containing information about the objective of the event
		 */
		PortalEvent(const PortalEventHeader& header) : mHeader(header) { }

		/**
		 * @return unique ID of the portal event
		 */
		const std::string& getID() const			{ return mHeader.mID; }

		/**
		 * @return unique ID of the sending / receiving portal
		 */
		const std::string& getPortalID() const		{ return mHeader.mPortalID; }

		/**
		 * @return type of the portal event, determines the effect
		 */
		const EPortalEventType& getType() const		{ return mHeader.mType; }

		/**
		 * Converts the portal event to a JSON string of API messages used for sending over the WebSocket server.
		 * @param outJSON the string that will contain the JSON after converting
		 * @param error should hold the error message when conversion fails
		 * @return whether the conversion was successful
		 */
		bool toAPIMessageJSON(std::string& outJSON, utility::ErrorState& error);

		/**
		 * Adds an API event to the portal event, relating to a portal item
		 * @param apiEvent the API event to add to the portal event
		 */
		void addAPIEvent(APIEventPtr apiEvent);

		/**
		 * @return the number of API events inside this portal event
		 */
		int getCount() const { return static_cast<int>(mAPIEvents.size()); }

		/**
		 * @return the API events of this portal event
		 */
		const APIEventConstIterator getAPIEvents() const { return APIEventConstIterator(mAPIEvents); }

		/**
		 * @param index the index of the API event
		 * @return an API event based on the given index, throws an exception when out of bounds
		 */
		const APIEvent* getAPIEvent(int index) const;

		/**
		 * @param index the index of the API event
		 * @return an API event based on the given index, throws an exception when out of bounds
		 */
		APIEvent* getAPIEvent(int index);

		/**
		 * Array [] subscript operator
		 * @return the API event at index
		 */
		APIEvent& operator[](std::size_t idx) { return *getAPIEvent(static_cast<int>(idx)); }

		/**
		 * Array [] subscript operator
		 * @return the API event at index
		 */
		const APIEvent& operator[](std::size_t idx) const { return *getAPIEvent(static_cast<int>(idx)); }

	private:

		PortalEventHeader mHeader;				///< Object containing all portal event header information
		std::vector<APIEventPtr> mAPIEvents;	///< The API events that this portal event contains
	};

	using PortalEventPtr = std::unique_ptr<PortalEvent>;
}
