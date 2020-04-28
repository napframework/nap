#pragma once

// internal includes
#include "sequenceeventtypes.h"

// external includes
#include <nap/event.h>

namespace nap
{
	/**
	 * Base class for all sequence related events.
	 */
	class SequenceEvent : public Event
	{
		RTTI_ENABLE(Event)
	};


	/**
	 * Sequence event that contains a message.
	 */
	class SequenceEventString : public SequenceEvent
	{
		RTTI_ENABLE(SequenceEvent)
	public:
		SequenceEventString(const std::string& message) : mMessage(message) { }

		/**
		 * @return message associated with event
		 */
		const std::string& getMessage() const { return mMessage; }

	private:
		std::string mMessage;

	};

	using SequenceEventPtr = std::unique_ptr<SequenceEvent>;
}
