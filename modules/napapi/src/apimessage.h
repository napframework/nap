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
	 * The 'Name' of the message is used to find a matching method to call inside the running application.
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
		 * Converts this message, including all of it's arguments into an api event.
		 * This event is the runtime version of an api message.
		 * @return the newly created api event.
		 */
		APIEventPtr toAPIEvent();

		/**
		 * Converts this message into a JSON readable string
		 * @param outJSON the output of the conversion
		 * @param error contains the error if the message couldn't be converted into a string
		 */
		bool toJSON(std::string& outString, utility::ErrorState& error);

		std::vector<ResourcePtr<APIBaseValue>> mArguments;	///< Property: 'Arguments': All input arguments associated with this message
		std::string mName;									///< Property: 'Name': action associated with the message

	private:
		// When constructing this message from an api event the values are owned by this object.
		// This ensures that when the message is destructed the arguments are deleted.
		std::vector<std::unique_ptr<APIBaseValue>> mOwningArguments;
	};
}
