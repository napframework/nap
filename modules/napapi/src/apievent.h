/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * 
	 *		// Create the event
	 *		APIEventPtr progress_event = std::make_unique<APIEvent>("CacheProgress");
	 *
	 *		// Add an argument
	 *		progress_event->addArgument<APIInt>("Percentage", ++curr_pro);
	 * 
	 *		// Dispatch to possible listeners
	 *		mAPIService->dispatchEvent(std::move(progress_event));
	 */
	class NAPAPI APIEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		using ArgumentConstIterator = utility::UniquePtrConstVectorWrapper<APIArgumentList, APIArgument*>;

		/**
		 * Construct a new api event with the given name. A unique id is generated.
		 * @param name name of this call
		 */
		APIEvent(const std::string& name);

		/**
		 * Construct a new api event with the given name. A unique id is generated.
		 * @param name name of this call
		 */
		APIEvent(std::string&& name);

		/**
		 * Construct a new api event with the given name and unique id.
		 * Use this constructor to form a reply based on a previously received client request.
		 * The uuid should match the uuid of the request. This allows the client to match call id's.
		 * @param name name of this call
		 * @param id unique identifier of this call
		 */
		APIEvent(const std::string& name, const std::string& id);

		/**
		 * Construct a new api event with the given name and unique id.
		 * Use this constructor to form a reply based on a previously received client request.
		 * The uuid should match the uuid of the request. This allows the client to match call id's.
		 * @param name identifier of this call
		 * @param id unique identifier of this call
		 */
		APIEvent(std::string&& name, std::string&& id);

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
		APIArgument* addArgument(const std::string& name, Args&&... args);

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
		 *	@return the arguments of this api event
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
		 * Returns this event as an event of type T.
		 * Example: ws_event = api_event.to<APIWebSocketEvent>();
		 * For this to work this event must be an event of type T.
		 * Asserts if the event isn't of type T.
		 * @return This event as T.
		 */
		template<typename T>
		const T& to() const;

		/**
		 * Returns this event as an event of type T.
		 * Example: ws_event = api_event.to<APIWebSocketEvent>();
		 * For this to work this event must be an event of type T.
		 * Asserts if the event isn't of type T.
		 * @return This event as T.
		 */
		template<typename T>
		T& to();

		/**
		 * Array [] subscript operator
		 * @return the api argument at index
		 */
		APIArgument& operator[](std::size_t idx)					{ return *getArgument(static_cast<int>(idx)); }

		/**
		 * Array [] subscript operator
		 * @return the api argument at index
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
	APIArgument* nap::APIEvent::addArgument(const std::string& name, Args&&... args)
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


	template<typename T>
	const T& nap::APIEvent::to() const
	{
		const T* return_p = nullptr;
		if (this->get_type().is_derived_from<T>())
			return_p = reinterpret_cast<const T*>(this);
		assert(return_p != nullptr);
		return *return_p;
	}


	template<typename T>
	T& nap::APIEvent::to()
	{
		T* cast_event = rtti_cast<T>(this);
		assert(cast_event != nullptr);
		return *cast_event;
	}

}
