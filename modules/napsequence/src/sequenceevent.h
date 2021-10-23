/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include <nap/event.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for all sequence related events.
	 */
	class NAPAPI SequenceEventBase : public Event
	{
		RTTI_ENABLE(Event)
	public:
        virtual ~SequenceEventBase(){};
        
		/**
		 * checks wether this event is of type T
		 * @tparam T the event type
		 * @return true if event is derived from T
		 */
		template<typename T>
		bool isEventType()
		{
			return RTTI_OF(T) == this->get_type();
		}

		/**
		 * Returns derived class instance reference
		 * @tparam T the type of derived class
		 * @return reference to derived class
		 */
		template<typename T>
		T& getEventType()
		{
			assert(this->get_type() == RTTI_OF(T)); // type mismatch
			return static_cast<T&>(*this);
		}
	};

	/**
	 * SequenceEvent is an event that holds a value of type T
	 * @tparam T the value type
	 */
	template<typename T>
	class SequenceEvent : public SequenceEventBase
	{
		RTTI_ENABLE(SequenceEventBase)
	public:
		/**
		 * Constructor
		 * @param value reference to value, is copied
		 */
		SequenceEvent(const T& value) : mValue(value) { }

		/**
		 * @return value of event
		 */
		const T& getValue() const { return mValue; }

	private:
		// the value
		T mValue;
	};

	using SequenceEventPtr = std::unique_ptr<SequenceEventBase>;

	//////////////////////////////////////////////////////////////////////////
	// Definitions of all supported events
	//////////////////////////////////////////////////////////////////////////

	using SequenceEventString = SequenceEvent<std::string>;
	using SequenceEventFloat = SequenceEvent<float>;
	using SequenceEventInt = SequenceEvent<int>;
	using SequenceEventVec2 = SequenceEvent<glm::vec2>;
	using SequenceEventVec3 = SequenceEvent<glm::vec3>;
}
