#pragma once

// Local  Includes
#include "apiargument.h"
#include "apisignature.h"

// External Includes
#include <nap/event.h>
#include <utility/uniqueptrvectoriterator.h>
#include <mathutils.h>

namespace nap
{
	using APIArgumentList = std::vector<std::unique_ptr<APIArgument>>;

	/**
	 * Input / Output message that is used at run-time to describe an api message.
	 * Contains a list of API arguments. Every api argument carries a single value or list of values.
	 * This event is created and given to the application by the APIService when an external request is made.
	 * To dispatch an event to an external environment call APIService::dispatchEvent after construction of this event. 
	 *
	 * Example: 	
	 * // Create the event
	 * APIEventPtr progress_event = std::make_unique<APIEvent>("CacheProgress");
	 *
	 * // Add an argument
	 * progress_event->addArgument<APIInt>("Percentage", ++curr_pro);
	 * 
	 * // Dispatch to possible listeners
	 * mAPIService->dispatchEvent(std::move(progress_event));
	 */
	class NAPAPI APIEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		using ArgumentConstIterator = utility::UniquePtrConstVectorWrapper<APIArgumentList, APIArgument*>;

		/**
		 * Default constructor
		 */
		APIEvent();

		/**
		 * Every API call needs to be associated with an action
		 * @param name name of this call
		 */
		APIEvent(const std::string& name);

		/**
		 * Move constructor
		 * Every API call needs to be associated with an action
		 * @param name name of this call
		 */
		APIEvent(const std::string&& name);

		/**
		 * Every API call needs to be associated with an action
		 * @param name name of this call
		 * @param id unique identifier of this call
		 */
		APIEvent(const std::string& name, const std::string& id);

		/**
		 * Move constructor
		 * Every API call needs to be associated with an action
		 * @param name identifier of this call
		 * @param id unique identifier of this call
		 */
		APIEvent(const std::string&& name, const std::string&& id);

		/**
		 * @return name (action) of this call	
		 */
		const std::string& getName() const							{ return mName; }

		/**
		 * @return id of this call
		 */
		const std::string& getID() const							{ return mID; }

		/**
		 * Adds an api argument with an id to this event where T is of type APIValue and 'args' the actual value, for example: 0.0f etc.
		 * To add a float as an argument call: addArgument<APIFloat>("drag", 1.0f).
		 * @param name the name of the newly created api value.
		 * @param args the template arguments used for constructing the argument. In case of an APIFloat the argument could be 1.0f etc.
		 * @return the newly created and added argument
		 */
		template<typename T, typename... Args>
		APIArgument* addArgument(const std::string&& name, Args&&... args);

		/**
		 * Adds an api argument to this event based on the given api value.
		 * @param value the api value to add as an argument.
		 * @return the added value as api argument.
		 */
		APIArgument* addArgument(std::unique_ptr<APIBaseValue> value);

		/**
		 * @return the number of arguments associated with this event
		 */
		int getCount() const										{ return static_cast<int>(mArguments.size()); }

		/**
		 *	@return the arguments of this osc event
		 */
		const ArgumentConstIterator getArguments() const			{ return ArgumentConstIterator(mArguments); }

		/**
		 * @return an argument based on the given index
		 * @param index the index of the argument, will throw an exception when out of bounds
		 */
		const APIArgument* getArgument(int index) const;

		/**
		 * @return an argument based on the given index
		 * @param index the index of the argument
		 */
		APIArgument* getArgument(int index);

		/**
		 * If the api arguments and order of arguments matches the given api signature.
		 * @param signature the method signature to validate
		 * @return if this event matches the given api signature.
		 */
		bool matches(const nap::APISignature& signature) const;

		/**
		 * Array [] subscript operator
		 * @return the osc argument at index
		 */
		APIArgument& operator[](std::size_t idx)					{ return *getArgument(static_cast<int>(idx)); }

		/**
		 * Array [] subscript operator
		 * @return the osc argument at index
		 */
		const APIArgument& operator[](std::size_t idx) const		{ return *getArgument(static_cast<int>(idx)); }

	private:
		std::string mName;				///< Name of the action associated with this call
		APIArgumentList mArguments;		///< All the arguments associated with the event
		std::string mID;				///< Unique ID of the api event
	};

	using APIEventPtr = std::unique_ptr<nap::APIEvent>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	APIArgument* nap::APIEvent::addArgument(const std::string&& name, Args&&... args)
	{
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(nap::APIBaseValue)));

		// Create value
		std::unique_ptr<T> value = std::make_unique<T>(name, std::forward<Args>(args)...);
		
		// Assign unique id
		value->mID = math::generateUUID();

		// Create argument and move value
		std::unique_ptr<APIArgument> argument = std::make_unique<APIArgument>(std::move(value));

		mArguments.emplace_back(std::move(argument));
		return mArguments.back().get();
	}
}
