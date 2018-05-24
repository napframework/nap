#pragma once

#include "rtti/rtti.h"
#include "utility/dllexport.h"

namespace nap
{
	/**
	 * An event is used to signal the occurrence of an action to the system
	 * There are many types of events: input (keyboard, mouse), osc, midi etc.
	 * Derive from this class to implement a new event type
	 */
	class NAPAPI Event
	{
		RTTI_ENABLE()
	public:
		// Default construction / destruction
		virtual ~Event() = default;
		Event() = default;

		// Disable copy
		Event(Event&) = delete;
		Event& operator=(const Event&) = delete;
	};

	/**
	 * Most events are wrapped in a unique pointer to manage lifetime automatically
	 */
	using EventPtr = std::unique_ptr<nap::Event>;
}
