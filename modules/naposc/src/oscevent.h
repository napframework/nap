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
		T* addArgument(Args&&... args);

		/**
		 * Adds an OSCArgument of type OSCValue<T> to this event
		 * This is a utility function that wraps addArgument based on it's contained value type
		 * Note that only registered OSC value types are considered valid
		 */
		template<typename T, typename... Args>
		OSCValue<T>* addValue(Args&&... args);

		/**
		 * @return the number of arguments associated with this event
		 */
		int numberOfArguments()									{ return static_cast<int>(mArguments.size()); }

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

		/**
		 * @return an argument @index of type T. Returns nullptr when argument is not of type T
		 * @param index the index of the argument
		 */
		template<typename T>
		const T* getArgument(int index) const;

		/**
		 * @return the value associated with an OSValueArgument of type T
		 * @param index the index of the argument
		 * If the argument is not of type value or the wrong type of value this call might lead to
		 * unexpected results
		 */
		template<typename T>
		T getValue(int index) const;

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

	template<typename T>
	const T* nap::OSCEvent::getArgument(int index) const
	{
		// Get argument and validate
		const OSCArgument& current_arg = getArgument(index);
		if (!(current_arg.get_type().is_derived_from(RTTI_OF(T))))
		{
			assert(false);
			return nullptr;
		}
		return &(static_cast<const T&>(current_arg));
	}

	template<typename T>
	T nap::OSCEvent::getValue(int index) const
	{
		const OSCArgument& current_arg = getArgument(index);
		assert(current_arg.get_type().is_derived_from(RTTI_OF(OSCValue<T>)));
		return static_cast<const OSCValue<T>&>(current_arg).mValue;
	}

	using OSCEventPtr = std::unique_ptr<nap::OSCEvent>;
}
