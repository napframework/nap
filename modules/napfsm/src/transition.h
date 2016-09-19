#pragma once
#include "state.h"
#include <nap.h>

namespace nap
{
	class StateMachineComponent;

	class Transition : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		Transition() = default;

		void setSrcState(State& state) { mSrcState = &state; }
		void setDstState(State& state) { mDstState = &state; }
		void setStates(State& fromState, State& toState)
		{
			setSrcState(fromState);
			setDstState(toState);
		}

		State* getSrcState() const { return mSrcState; }
		State* getDstState() const { return mDstState; }

		StateMachineComponent& getStateMachine();

        bool isInterruptible() const;
        void setInterruptible(bool b) { mIsInterruptible = b; }
	private:
        bool mIsInterruptible = false;
		State* mDstState = nullptr;
		State* mSrcState = nullptr;
	};

	// Will execute when its action is sent through the state machine
	class MessageTransition : public Transition
	{
		RTTI_ENABLE_DERIVED_FROM(Transition)
	public:
		MessageTransition() : Transition() {}

		Attribute<std::string> messageString = {this, "ActionString"};
	};

    class AutoTransition : public Transition
    {
        RTTI_ENABLE_DERIVED_FROM(Transition)
    public:
        AutoTransition() : Transition() {}

    };

//
}

RTTI_DECLARE(nap::Transition)
RTTI_DECLARE(nap::MessageTransition)
RTTI_DECLARE(nap::AutoTransition)
