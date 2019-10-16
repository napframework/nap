#pragma once

// Local includes
#include "oscargument.h"

// External Includes
#include <nap/event.h>
#include <nap/numeric.h>
#include <utility/uniqueptrvectoriterator.h>

namespace nap
{
	using OSCArgumentList = std::vector<std::unique_ptr<OSCArgument>>;

	/**
	 * Represents a generic OSC event. An OSC event has an address and a set of arguments (values) associated with it.
	 * This event can be constructed by a client to be send over or evaluated when received.
	 * When constructing this event, the given address must start with a '/' character!
	 * Use the array [] overload to access the individual osc arguments.
	 */
	class NAPAPI OSCEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		using ArgumentConstIterator = utility::UniquePtrConstVectorWrapper<OSCArgumentList, OSCArgument*>;

        OSCEvent() = delete;
        
		/**
		 * OSCEvent constructor
		 * @param address the address associated with this osc event
		 */
		OSCEvent(const std::string& address);

		/**
		 * OSCEvent constructor
		 * @param address the address associated with this osc event
		 */
		OSCEvent(const std::string&& address);

		/**
		 * @return this event's OSC address
		 */
		const std::string& getAddress() const								{ return mAddress; }

		/**
		 * Adds an OSCArgument to this event. The template type must be of type OSCBaseValue.
		 * The arguments are used to construct the OSCValue, for example: 
		 *
		 *		addArgument<OSCFloat>(1.0f) 
		 *		addArgument<OSCString>("ola!")
		 *
		 * @args the arguments that are used for constructing the specified OSCValue. 
		 * @return the newly created and added argument
		 */
		template<typename T, typename... Args>
		OSCArgument* addArgument(Args&&... args);

		/**
		 * Adds an OSCArgument to this event. The argument wraps an OSCValue of type T, for example:
		 *
		 * addValue<float>(1.0f)	-> adds an OSCValue<float> 
		 * addValue<int>(1)			-> adds an OSCValue<int>.
		 *
		 * This is a utility function that wraps addArgument based on the given OSC value type.
		 * Note that only registered OSC value types are considered valid.
		 * @param args the value that is used to construct the OSCValue.
		 * @return the newly created and added argument.
		 */
		template<typename T, typename... Args>
		OSCArgument* addValue(Args&&... args);

		/**
		 * Adds an OSCArgument that holds a string. 
		 * This is a utility function that wraps addArgument
		 * @param string the string to give to the argument
		 * @return the newly created and added argument.
		 */
		OSCArgument* addString(const std::string& string);

		/**
		 * @return the number of arguments associated with this event
		 */
		int getCount() const												{ return static_cast<int>(mArguments.size()); }

		/**
		 *	@return the arguments of this osc event
		 */
		const ArgumentConstIterator getArguments() const					{ return ArgumentConstIterator(mArguments); }

		/**
		 * @return an argument based on index
		 * @param index the index of the argument, will throw an exception when out of bounds
		 */
		const OSCArgument* getArgument(int index) const;

		/**
		 * @return an argument based on index
		 * @param index the index of the argument
		 */
		OSCArgument* getArgument(int index);
        
		/**
		 * @return the size of the osc event in bytes
		 * This includes the size of the arguments together with the name of this event
		 */
		std::size_t getSize() const;

		/**
		 * Array [] subscript operator
		 * @return the osc argument at index
		 */
		OSCArgument& operator[](std::size_t idx)							{ return *getArgument(static_cast<int>(idx)); }
		
		/**
		 * Array [] subscript operator
		 * @return the osc argument at index
		 */
		const OSCArgument& operator[](std::size_t idx) const				{ return *getArgument(static_cast<int>(idx)); }

	private:
		OSCArgumentList mArguments;							// All the arguments associated with the event
		std::string mAddress;								// The osc event address
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
		return addArgument<nap::OSCValue<T>>(std::forward<Args>(args)...);
	}

	using OSCEventPtr = std::unique_ptr<nap::OSCEvent>;
}
