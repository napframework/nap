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
	 * Represents a message that can be given to or is send from a running NAP application.
	 * Every api message can be serialized from and to JSON and holds a list of API values.
	 * At runtime, an api message is converted into an api event, which is the runtime version of an api message.
	 * A message can also be created from an event, which in turn can be given to an external environment.
	 * Only messages that match an APISignature (as declared by the app) are forwarded to the application.
	 */
	class NAPAPI APIMessage : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~APIMessage();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Converts this message, including all arguments, into an api event.
		 * This event is the runtime version of an api message.
		 * @return the newly created api event.
		 */
		APIEventPtr toAPIEvent();

		std::vector<ResourcePtr<APIBaseValue>> mArguments;	///< Property: 'Arguments': All input arguments associated with this message
	};
}
