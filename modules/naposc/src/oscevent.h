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

		/**
		 * Adds an OSCArgument to this event
		 * @args the template arguments used for constructing the argument
		 * @return the newly created and added argument
		 */
		template<typename T, typename... Args>
		OSCArgument* addArgument(Args&&... args);

		/**
		 * Adds an OSCArgument that that is constructed with an OSCValue<T> as input argument
		 * This is a utility function that wraps addArgument based OSC value type
		 * Note that only registered OSC value types are considered valid
		 */
		template<typename T, typename... Args>
		OSCArgument* addValue(Args&&... args);

		/**
		 * @return the number of arguments associated with this event
		 */
		int getSize()											{ return static_cast<int>(mArguments.size()); }

		/**
		 * @return an argument based on @index
		 * @param index the index of the argument, will throw an exception when out of bounds
		 */
		const OSCArgument& getArgument(int index) const;

		/**
		 * @return an argument based on @index
		 * @param index the index of the argument
		 */
		OSCArgument& getArgument(int index);

		// Array subscript overloads
		OSCArgument& operator[](std::size_t idx)				{ return getArgument(static_cast<int>(idx)); }
		const OSCArgument& operator[](std::size_t idx) const	{ return getArgument(static_cast<int>(idx)); }

	private:
		std::vector<std::unique_ptr<OSCArgument>> mArguments;
	};

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	OSCArgument* nap::OSCEvent::addArgument(Args&&... args)
	{
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(nap::OSCBaseValue)));
		
		// Create value
		std::unique_ptr<T> value = std::make_unique<T>(std::forward<Args>(args)...);
		
		// Create argument and move value
		std::unique_ptr<OSCArgument> argument = std::make_unique<OSCArgument>(std::move(value));
		
		mArguments.emplace_back(std::move(argument));
		return mArguments.back().get();
	}

	template<typename T, typename... Args>
	OSCArgument* nap::OSCEvent::addValue(Args&&... args)
	{
		// Ensure that the value for the osc argument is valid
		if(RTTI_OF(nap::OSCValue<T>).empty())
		{
			assert(false);
			return nullptr;
		}
		return addArgument<nap::OSCValue<T>>(std::forward<Args>(args)...);
	}

	using OSCEventPtr = std::unique_ptr<nap::OSCEvent>;
}
