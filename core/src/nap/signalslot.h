#pragma once

#include <functional>
#include <set>
#include <vector>

namespace nap
{

	/**
	Manages a function object.
	Every event needs to be initialized with a function that can be called by other events
	The variable types specify the function arguments.
	Other events can be connected/disconnected to an event.
	**/
	template <typename... Args>
	class SignalSlotBase
	{
	public:
		// Forward Declares
		using Function = std::function<void(Args... args)>;

	public:
		// Construction
		SignalSlotBase(Function inFunction) : mFunction(inFunction) {}
		SignalSlotBase() {}

		// Destruction
		~SignalSlotBase();

        // TODO: REMOVE What is this? Similar to connect(function) ?
		void setFunction(Function func) { mFunction = func; }

		// Connection
		void connect(SignalSlotBase<Args...>& event);
		void disconnect(SignalSlotBase<Args...>& event);

		// connect a raw functionobject. Lifelong connection only, disconnection not possible.
        // TODO: Why is disconnection not possible??
		void connect(Function inFunction);

		// convenience method for lifelong connection in case of single parameter events
        // TODO: Why is disconnection not possible??
		template <typename U, typename F>
		void connect(U* parent, F memberFunction)
		{
			connect(std::bind(memberFunction, parent, std::placeholders::_1));
		}

		inline void operator()(Args... args) { trigger(std::forward<Args>(args)...); }

		// Trigger the event
		void trigger(Args... args);

	private:
		// Function pointer
		Function mFunction;

	protected:
		// Members
		std::set<SignalSlotBase<Args...>*> mCauses;  //< Signals
		std::set<SignalSlotBase<Args...>*> mEffects; //< Slots or Signals
		std::vector<Function> mFunctionEffects;		 //< function objects
	};


	/**
	A signal can be emitted, which results in all the connected events or slots being called.
	**/
	template <typename... Args>
	class Signal : public SignalSlotBase<Args...>
	{
	public:
		// Constructor
		Signal() : SignalSlotBase<Args...>(nullptr) {}

		// returns the number of connected slots and signals
		unsigned int getConnectionCount() { return SignalSlotBase<Args...>::mEffects.size(); }
	};


	/**
	@brief Slot

	A slot manages a function call that can be connected to a signal.
	A slot hides the connect / disconnect behavior of an event.
	**/
	template <typename... Args>
	class Slot : public SignalSlotBase<Args...>
	{
	public:
		//@name The function pointer needs to be redeclared public here because somehow the LLVM compiler does not
		//recognize it from the base class Event
		using Function = std::function<void(Args... args)>;

	public:
		//@name Construction
		Slot(Function inFunction) : SignalSlotBase<Args...>(inFunction) {}
		Slot() : SignalSlotBase<Args...>() {}

		//! This templated constructor can be used to initialize the slot with a member function with one single
		//! parameter
		template <typename U, typename F>
		Slot(U* parent, F memberFunction)
			: SignalSlotBase<Args...>(std::bind(memberFunction, parent, std::placeholders::_1))
		{
		}

		//! This templated constructor can be used to initialize the slot with a member function with one single
		//! parameter, last argument is a signal to connect to straightaway after construction
		template <typename U, typename F>
		Slot(U* parent, F memberFunction, Signal<Args...>& signal)
			: SignalSlotBase<Args...>(std::bind(memberFunction, parent, std::placeholders::_1))
		{
			signal.connect(*this);
		}

		//@name Remove members
        // TODO: Why? These two methods can just as well be declared in Signal alone
		void connect(SignalSlotBase<Args...>& event) = delete;
		void disconnect(SignalSlotBase<Args...>& event) = delete;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Implementations
	//////////////////////////////////////////////////////////////////////////

	template <typename... Args>
	SignalSlotBase<Args...>::~SignalSlotBase()
	{
		for (auto& cause : mCauses)
			cause->mEffects.erase(this);

		for (auto& effect : mEffects)
			effect->mCauses.erase(this);
	}

	template <typename... Args>
	void SignalSlotBase<Args...>::connect(SignalSlotBase<Args...>& event)
	{
		mEffects.emplace(&event);
		event.mCauses.emplace(this);
	}

	template <typename... Args>
	void SignalSlotBase<Args...>::connect(Function inFunction)
	{
		mFunctionEffects.emplace_back(inFunction);
	}

	template <typename... Args>
	void SignalSlotBase<Args...>::disconnect(SignalSlotBase<Args...>& event)
	{
		mEffects.erase(&event);
		event.mCauses.erase(this);
	}

	template <typename... Args>
	void SignalSlotBase<Args...>::trigger(Args... args)
	{
		if (mFunction)
			mFunction(std::forward<Args>(args)...);

		for (auto& effect : mEffects)
			effect->trigger(std::forward<Args>(args)...);

		for (auto& effect : mFunctionEffects)
			effect(std::forward<Args>(args)...);
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
