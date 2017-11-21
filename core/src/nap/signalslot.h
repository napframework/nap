#pragma once

#include <functional>
#include <set>
#include <vector>
#include <memory>

namespace nap
{
	template<typename... Args> class Slot;

	/**
	Manages a function object.
	Every event needs to be initialized with a function that can be called by other events
	The variable types specify the function arguments.
	Other events can be connected/disconnected to an event.
	**/
	template <typename... Args>
	class Signal final
	{
	public:
		using Function = std::function<void(Args... args)>;

		// Destruction
		~Signal();

		// Connection
		void connect(Signal<Args...>& signal);
		void disconnect(Signal<Args...>& signal);

		void connect(Slot<Args...>& slot);
		void disconnect(Slot<Args...>& slot);

		// Connect a raw function object. Lifelong connection only, disconnection not possible.
		void connect(const Function& inFunction);

		// Convenience method for lifelong connection in case of single parameter events
		template <typename U, typename F>
		void connect(U* object, F memberFunction)
		{
			connect(std::bind(memberFunction, object, std::placeholders::_1));
		}

		inline void operator()(Args... args) 
		{ 
			trigger(std::forward<Args>(args)...); 
		}

		// Trigger the event
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

		void setFunction(Function func) 
		{ 
			mFunction = func; 
		}

		void trigger(Args... args)
		{
			if (mFunction)
				mFunction(std::forward<Args>(args)...);
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
