#pragma once

// internal includes
#include "sequenceeventtypes.h"

// external includes
#include <nap/event.h>

namespace nap
{
	/**
	 * SequenceEvent
	 * Base class for events.
	 */
	class SequenceEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		/**
		 * getEventType
		 * @return returns this event type
		 */
		virtual const SequenceEventTypes::Types getEventType() const { return SequenceEventTypes::UNKOWN; }
	};

	/**
	 * SequenceEvent String
	 * Event holding a message ( string )
	 */
	class SequenceEventString : public SequenceEvent
	{
		RTTI_ENABLE(SequenceEvent)
	public:
		std::string mMessage; ///< Property: 'Message' string containing message

		/**
		 * getEventType
		 * @return returns this event type
		 */
		virtual const SequenceEventTypes::Types getEventType() const override
		{
			return SequenceEventTypes::STRING; 
		}
	};

	using SequenceEventPtr = std::unique_ptr<SequenceEvent>;
}
