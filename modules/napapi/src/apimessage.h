/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "apivalue.h"
#include "apievent.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents a message that can be given to a running NAP application by calling APIService::sendMessage().
	 * When doing so the message is converted into an api event, which is the runtime version of an api message.
	 * For convenience, an api event can also be converted into a message and therefore serialized to JSON.
	 * This allows for an easy exchange of messages from and to a NAP application.
	 *
	 * Note that the 'mID' of the message needs to be unique 
	 * when sending multiple messages as a bundle from an external environment.
	 * The 'Name' of the message is used to find a matching callback with the same name inside the running application.
	 * When the name of the message matches the signature of the callback that callback is invoked.
	 */
	class NAPAPI APIMessage : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		APIMessage() = default;

		// Destructor
		virtual ~APIMessage();

		/**
		 * Constructs this message based on the given api event.
		 * All arguments in the api events are copied into this object.
		 * @param apiEvent the event to convert into a message.
		 */
		APIMessage(const APIEvent& apiEvent);

		/**
		 * Constructs this message based on the given name
		 * @param name the name (action) associated with this message
		 */
		APIMessage(const std::string& name);

		/**
		 * Converts this message, including all of it's arguments into an api event of type T.
		 * This event is the runtime version of an api message.
		 * @param args extra input arguments given on construction to api-event of type T.
		 * @return the newly created api event of type T.
		 */
		template<typename T, typename... Args>
		std::unique_ptr<T> toEvent(Args&&... args);

		/**
		 * Converts this message into a JSON readable string
		 * @param outString the output of the conversion
		 * @param error contains the error if the message couldn't be converted into a string
		 * @return if the message converted to JSON successfully. 
		 */
		bool toJSON(std::string& outString, utility::ErrorState& error);

		std::vector<APIBaseValue*> mArguments;		///< Property: 'Arguments': All input arguments associated with this message
		std::string mName;							///< Property: 'Name': action associated with the message

	private:
		// When constructing this message from an api event the values are owned by this object.
		// This ensures that when the message is destructed the arguments are deleted.
		std::vector<std::unique_ptr<APIBaseValue>> mOwningArguments;

		/**
		 * Copies all arguments present in this message to the given api event.
		 */
		void copyArguments(APIEvent& apiEvent);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	std::unique_ptr<T> nap::APIMessage::toEvent(Args&&... args)
	{
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(APIEvent)));
		std::unique_ptr<T> ptr = std::make_unique<T>(mName, mID, std::forward<Args>(args)...);
		copyArguments(*ptr);
		return ptr;
	}
}
