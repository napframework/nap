/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <functional>
#include <set>
#include <vector>
#include <memory>
#include <iostream>

// Pybind includes
#include "python.h"

namespace nap
{
    
    // Forward declarations
	template<typename... Args> class Slot;
    
    /**
     * A callable signal to which slots, functions or other signals can be connected to provide loose coupling.
     * The signal variadic template arguments to be able to work with different sets of arguments.
     */
	template <typename... Args>
	class Signal final
	{
	public:
		using Function = std::function<void(Args... args)>;

		// Destruction
		~Signal();

		// Connection
        
        /**
         * Connect to another signal that can call other functions, slots or signals in turn
         */
		void connect(Signal<Args...>& signal);
        
        /**
         * Disconnect from another signal
         */
		void disconnect(Signal<Args...>& signal);

        /**
         * Connect to a slot with similar signature. A slot is an object managing a function to be called.
         * Advantages of connecting to a slot instead of a raw function:
         * - the slot disconnects itself on destruction, so it's still safe to call the signal afterwards
         * - the slot can be disconnected from the signal
         */
		void connect(Slot<Args...>& slot);
        
        /**
         * Disconnect from a slot
         */
		void disconnect(Slot<Args...>& slot);

        /**
         * Connect a raw function object.
         * Note that when any captured data in a connected function is deleted this signal will be unsafe to call.
         * Also disconnecting a raw function is not possible.
         * Connecting a function is mainly only good practice when the scope of the signal is the same as the function's.
         */
		void connect(const Function& inFunction);

#ifdef NAP_ENABLE_PYTHON
        /**
         * Connect a function from a pybind11 python module. Internally the python function is wrapped in a function object.
         */
        void connect(const pybind11::function pythonFunction);
#endif // NAP_ENABLE_PYTHON

        /**
         * Convenience method to connect a member function with one argument.
         */
		template <typename U, typename F>
		void connect(U* object, F memberFunction)
		{
			connect(std::bind(memberFunction, object, std::placeholders::_1));
		}

        /**
         * Call operator to trigger the signal to be emitted.
         */
		inline void operator()(Args... args) 
		{ 
			trigger(std::forward<Args>(args)...); 
		}

        /**
         * Trigger the signal to be emitted
         */
		void trigger(Args... args);

	private:
		void addCause(Signal<Args...>& event);
		void removeCause(Signal<Args...>& event);

	private:

		// Data acts like a variant type with a few known types.  
		// The reason for this approach is that we can use a single list
		// of elements in Signal instead of having a separate list for each.
		// This saves out a number of dynamic memory allocations and a large
		// overhead in member data, without introducing any virtual function
		// calls.
		struct Data
		{
			union 
			{
				struct
				{
					Signal<Args...>* mSignal;
				} Signal;

				struct
				{
					Slot<Args...>* mSlot;
				} Slot;
			};

			enum class EType : uint8_t
			{
				SignalCause,
				SignalEffect,
				SlotEffect
			};
			EType mType;
		};

		// Members
		std::vector<Data>						mData;					// Signal/slot effects, Signal causes
		std::unique_ptr<std::vector<Function>>	mFunctionEffects;		// function objects
	};


	/**
	@brief Slot

	A slot manages a function call that can be connected to a signal.
	A slot hides the connect / disconnect behavior of an event.
	**/
	template <typename... Args>
	class Slot final
	{
	public:
		using Function = std::function<void(Args... args)>;

	public:
		//@name Construction
		Slot() = default;

		Slot(Function inFunction) : 
			mFunction(inFunction) 
		{
		}

		//! This templated constructor can be used to initialize the slot with a member function with one single
		//! parameter
		template <typename U, typename F>
		Slot(U* parent, F memberFunction) : 
			mFunction(std::bind(memberFunction, parent, std::placeholders::_1))
		{
		}

		//! This templated constructor can be used to initialize the slot with a member function with one single
		//! parameter, last argument is a signal to connect to straightaway after construction
		template <typename U, typename F>
		Slot(U* parent, F memberFunction, Signal<Args...>& signal) : 
			mFunction(std::bind(memberFunction, parent, std::placeholders::_1))
		{
			signal.connect(*this);
		}

		~Slot()
		{
			disconnect();
		}

		/**
		 * Disconnects the slot from all signals it is connected to
		 */
		void disconnect();

		void setFunction(Function func) 
		{ 
			mFunction = func; 
		}

		void trigger(Args... args)
		{
			if (mFunction)
				mFunction(std::forward<Args>(args)...);
		}

		void copyCauses(const Slot& rhs)
		{
			for (auto cause : rhs.mCauses)
				cause->connect(*this);
		}

	private:
		template<typename... Args_> friend class Signal;

		void addCause(Signal<Args...>& event);
		void removeCause(Signal<Args...>& event);

	private:
		typedef std::vector<Signal<Args...>*> SignalList;
		Function mFunction;
		SignalList mCauses;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Implementations
	//////////////////////////////////////////////////////////////////////////

	template <typename... Args>
	Signal<Args...>::~Signal()
	{
		for (auto& data : mData)
		{
			if (data.mType == Data::EType::SignalCause)
				data.Signal.mSignal->disconnect(*this);
			else if (data.mType == Data::EType::SignalEffect)
				data.Signal.mSignal->removeCause(*this);
			else
				data.Slot.mSlot->removeCause(*this);
		}
	}

	template <typename... Args>
	void Signal<Args...>::addCause(Signal<Args...>& signal)
	{
		Data data;
		data.mType = Data::EType::SignalCause;
		data.Signal.mSignal = &signal;
		mData.push_back(data);
	}

	template <typename... Args>
	void Signal<Args...>::removeCause(Signal<Args...>& event)
	{
		for (int index = 0; index < mData.size(); ++index)
		{
			Data& value = mData[index];
			if (value.mType == Data::EType::SignalCause && value.Signal.mSignal == &event)
			{
				mData.erase(mData.begin() + index);
				break;
			}
		}
	}
    
	template <typename... Args>
	void Signal<Args...>::connect(Signal<Args...>& signal)
	{
		Data data;
		data.mType = Data::EType::SignalEffect;
		data.Signal.mSignal = &signal;
		mData.push_back(data);

		signal.addCause(*this);
	}

	template <typename... Args>
	void Signal<Args...>::disconnect(Signal<Args...>& signal)
	{
		for (int index = 0; index < mData.size(); ++index)
		{
			Data& value = mData[index];
			if (value.mType == Data::EType::SignalEffect && value.Signal.mSignal == &signal)
			{
				value.Signal.mSignal->removeCause(*this);
				mData.erase(mData.begin() + index);
				break;
			}
		}
	}

	template <typename... Args>
	void Signal<Args...>::connect(Slot<Args...>& slot)
	{
		Data data;
		data.mType = Data::EType::SlotEffect;
		data.Slot.mSlot = &slot;
		mData.push_back(data);

		slot.addCause(*this);
	}

	template <typename... Args>
	void Signal<Args...>::disconnect(Slot<Args...>& slot)
	{
		for (int index = 0; index < mData.size(); ++index)
		{
			Data& data = mData[index];
			if (data.mType == Data::EType::SlotEffect && data.Slot.mSlot == &slot)
			{
				data.Slot.mSlot->removeCause(*this);
				mData.erase(mData.begin() + index);
				break;
			}
		}
	}
    
    
	template <typename... Args>
	void Signal<Args...>::connect(const Function& inFunction)
	{
		if (!mFunctionEffects)
			mFunctionEffects = std::make_unique<std::vector<Function>>();

		mFunctionEffects->emplace_back(inFunction);
	}
    
#ifdef NAP_ENABLE_PYTHON   
    template <typename... Args>
    void Signal<Args...>::connect(const pybind11::function pythonFunction)
    {
        Function func = [pythonFunction](Args... args)
        {
            try
            {
                pythonFunction(pybind11::cast(std::forward<Args>(args)..., std::is_lvalue_reference<Args>::value
                                              ? pybind11::return_value_policy::reference : pybind11::return_value_policy::automatic_reference)...);
            }
            catch (const pybind11::error_already_set& err)
            {
                auto message = std::string("Runtime python error while executing signal: ") + std::string(err.what());
                
                // TODO It would be preferable to log python error message using the nap logger.
                // Unfortunately the logger is not accessible in signalslot.h though because it uses Signals itself.
                std::cout << message << std::endl;
            }
        };
        connect(func);
    }
#endif // NAP_ENABLE_PYTHON
    
    
	template <typename... Args>
	void Signal<Args...>::trigger(Args... args)
	{
		for (auto& data : mData)
		{
			if (data.mType == Data::EType::SignalEffect)
				data.Signal.mSignal->trigger(std::forward<Args>(args)...);
			else if (data.mType == Data::EType::SlotEffect)
				data.Slot.mSlot->trigger(std::forward<Args>(args)...);
		}

		if (mFunctionEffects)
			for (auto& effect : *mFunctionEffects)
				effect(std::forward<Args>(args)...);
	}

	template <typename... Args>
	void Slot<Args...>::disconnect()
	{
		for (int index = mCauses.size() - 1; index >= 0; --index)
			mCauses[index]->disconnect(*this);
	}

	template <typename... Args>
	void Slot<Args...>::addCause(Signal<Args...>& event)
	{
		mCauses.push_back(&event);
	}

	template <typename... Args>
	void Slot<Args...>::removeCause(Signal<Args...>& event)
	{
		for (typename SignalList::iterator pos = mCauses.begin(); pos != mCauses.end(); ++pos)
		{
			if ((*pos) == &event)
			{
				mCauses.erase(pos);
				break;
			}
		}
	}

} // End Namespace nap


//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////


// Creates a slot with the given @NAME and @TYPE and binds it to @FUNCTION using a lambda
#define NSLOT(NAME, TYPE, FUNCTION) nap::Slot<TYPE> NAME = {[&](TYPE inValue) -> void { FUNCTION(inValue); }};

// Creates a slot with @NAME and @TYPE without binding it to a function
#define CREATE_SLOT(NAME, TYPE) nap::Slot<TYPE> NAME;

// Binds the already defined slot with @NAME and @TYPE to @FUNCTION
#define BIND_SLOT(NAME, TYPE, FUNCTION) NAME([&](TYPE inValue) -> void { FUNCTION(inValue); })

// SIGNAL MACRO
#define NSIGNAL(NAME, TYPE) nap::Signal<TYPE> NAME;
