#pragma once

// Local  Includes
#include "apiargument.h"

// External Includes
#include <nap/event.h>
#include <utility/uniqueptrvectoriterator.h>

namespace nap
{
	/**
	 * Base class associated of all NAP API related events.	
	 */
	class NAPAPI APIEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	using APIArgumentList = std::vector<std::unique_ptr<APIArgument>>;

	/**
	 * API event that is created when calling NAP from an external environment.
	 * It contains a number of API arguments that hold specific values.
	 * The application decides what to do with these events.
	 * These events are forwarded to the running app by the APIService.
	 */
	class NAPAPI APICallEvent : public APIEvent
	{
		RTTI_ENABLE(APIEvent)
	public:
		using ArgumentConstIterator = utility::UniquePtrConstVectorWrapper<APIArgumentList, APIArgument*>;

		/**
		 * Default constructor
		 */
		APICallEvent() = default;

		/**
		 * Every API call needs to be associated with an action
		 * @param action name of the action associated with this call
		 */
		APICallEvent(const std::string& action);

		/**
		 * Every API call needs to be associated with an action
		 * @param action name of the action associated with this call
		 */
		APICallEvent(const std::string&& action);

		/**
		 * @return action name of this call	
		 */
		const std::string& getAction() const						{ return mActionName; }

		/**
		 * Adds an api argument to this event where VT needs to be of type APIValue.
		 * @args the template arguments used for constructing the argument
		 * @return the newly created and added argument
		 */
		template<typename T, typename... Args>
		APIArgument* addArgument(Args&&... args);

		/**
		 * @return the number of arguments associated with this event
		 */
		int getCount() const										{ return static_cast<int>(mArguments.size()); }

		/**
		*	@return the arguments of this osc event
		*/
		const ArgumentConstIterator getArguments() const			{ return ArgumentConstIterator(mArguments); }

		/**
		* @return an argument based on @index
		* @param index the index of the argument, will throw an exception when out of bounds
		*/
		const APIArgument* getArgument(int index) const;

		/**
		* @return an argument based on @index
		* @param index the index of the argument
		*/
		APIArgument* getArgument(int index);

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
		std::string mActionName;		///< Name of the action associated with this call
		APIArgumentList mArguments;		// All the arguments associated with the event
	};

	using APICallEventPtr = std::unique_ptr<nap::APICallEvent>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	APIArgument* nap::APICallEvent::addArgument(Args&&... args)
	{
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(nap::APIBaseValue)));

		// Create value
		std::unique_ptr<T> value = std::make_unique<T>(std::forward<Args>(args)...);

		// Create argument and move value
		std::unique_ptr<APIArgument> argument = std::make_unique<APIArgument>(std::move(value));

		mArguments.emplace_back(std::move(argument));
		return mArguments.back().get();
	}
}
