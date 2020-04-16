#pragma once

#include <nap/event.h>

namespace nap
{
	namespace SequenceEventTypes
	{
		enum Types
		{
			STRING,
			UNKOWN
		};
	}

	/**
	 *
	 */
	class SequenceEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		virtual const SequenceEventTypes::Types getEventType() const { return SequenceEventTypes::UNKOWN; }
	};

	/**
	 *
	 */
	class SequenceEventString : public SequenceEvent
	{
		RTTI_ENABLE(SequenceEvent)
	public:
		std::string mMessage;

		virtual const SequenceEventTypes::Types getEventType() const { return SequenceEventTypes::STRING; }
	};

	using SequenceEventPtr = std::unique_ptr<SequenceEvent>;
}
