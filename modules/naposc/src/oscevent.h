#pragma once

// Local includes
#include "oscargument.h"

// External Includes
#include <nap/event.h>
#include <nap/configure.h>

namespace nap
{
	/**
	 * Generic OSC event
	 * An OSC event has an address and a set of arguments associated with it
	 * This event can be constructed by a client to be send over or evaluated when received
	 */
	class NAPAPI OSCEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		/**
		 * OSCEvent constructor
		 * @param address the address associated with this osc event
		 */
		OSCEvent(const std::string& address);

		// The osc event address
		std::string mAddress;

		std::vector<std::unique_ptr<OSCArgument>>& getArguments() { return mArguments; }

		/**
		 * Adds an OSCArgument to this event
		 * @args the template arguments used for constructing the argument
		 * @return the newly created and added argument
		 */
		template<typename T, typename... Args>
		T* addArgument(Args&&... args);

		/**
		 * Adds an OSCArgument of type OSCValue<T> to this event
		 * This is a utility function that wraps addArgument based on it's contained value type
		 * Note that only registered OSC value types are considered valid
		 */
		template<typename T, typename... Args>
		OSCValue<T>* addValue(Args&&... args);

	private:
		std::vector<std::unique_ptr<OSCArgument>> mArguments;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	T* nap::OSCEvent::addArgument(Args&&... args)
	{
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(nap::OSCArgument)));
		mArguments.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		return static_cast<T*>(mArguments.back().get());
	}

	template<typename T, typename... Args>
	OSCValue<T>* nap::OSCEvent::addValue(Args&&... args)
	{
		// Ensure that the value for the osc argument is valid
		if(RTTI_OF(nap::OSCValue<T>).empty())
		{
			assert(false);
			return nullptr;
		}
		return addArgument<nap::OSCValue<T>>(std::forward<Args>(args)...);
	}
}

using OSCEventPtr = std::unique_ptr<nap::OSCEvent>;
