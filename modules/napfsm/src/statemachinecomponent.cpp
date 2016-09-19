#include "statemachinecomponent.h"
#include "statemachineservice.h"

RTTI_DEFINE(nap::StateMachineComponent)

namespace nap
{
	StateMachineComponent::StateMachineComponent() : EventHandlerComponent()
	{
		mAnyState = &addChild<State>("__AnyState__");
		mHistoryStateStub = &addChild<State>("__HistoryState__");
		added.connect(
			[&](Object& parent) { getRoot().getCore().getService<StateMachineService>()->registerComponent(*this); });

		removed.connect(
			[&](Object& parent) { getRoot().getCore().getService<StateMachineService>()->removeComponent(*this); });
	}

	void StateMachineComponent::initialize()
	{
		// Execute initial state
		mNextState = mCurrentState;
		enterNextStateNow();
	}


	State& StateMachineComponent::addState(const std::string& name)
	{
		State& state = addChild<State>(name);
		if (!mCurrentState) mCurrentState = &state;

		// Forward signals to StateMachine listeners
		state.entering.connect([&](State& state) { stateEntering(state); });
		state.entered.connect([&](State& state) { stateEntered(state); });
		state.exiting.connect([&](State& state) { stateExiting(state); });
		state.exited.connect([&](State& state) { stateExited(state); });

		if (!initialState.getValueRef().empty()) setInitialState(state);

		return state;
	}

	nap::State* StateMachineComponent::getState(const std::string& name)
	{
		State* s = getChild<State>(name);
		return s;
	}

	Transition* StateMachineComponent::addAutoTransition(State& fromState, State& toState)
	{
		auto& trans = addChild<AutoTransition>(fromState.getName() + "_to_" + toState.getName());
		trans.setStates(fromState, toState);
		return &trans;
	}


	Transition* StateMachineComponent::addMessageTransition(State& fromState, State& toState,
															const std::string& message)
	{
		auto& trans = addChild<MessageTransition>(fromState.getName() + "_to_" + toState.getName());
		trans.setStates(fromState, toState);
		trans.messageString.setValue(message);
		return &trans;
	}

	nap::Transition* StateMachineComponent::addMessageTransition(State& toState, const std::string& message)
	{
		return addMessageTransition(getAnyState(), toState, message);
	}


	void StateMachineComponent::enterStatePrepare(State& state)
	{
		// We're not entering the next state right away, store it
		if (&state == mHistoryStateStub) {
			// If history state is next, get the actual last state
			assert(mHistoryState);
			mNextState = mHistoryState;
		} else {
			mNextState = &state;
		}

		if (mCurrentState) {
			mCurrentState->exit();
		} else { // No current state, transition immediately
			enterNextStateNow();
		}
	}

	void StateMachineComponent::enterNextStateNow()
	{
		assert(mNextState);
		mHistoryState = mCurrentState;
		mCurrentState = mNextState;
		mNextState = nullptr;
		mCurrentState->enter();

        auto autoTransition = findAutoTransition(*mCurrentState);
        if (autoTransition)
            executeTransition(autoTransition);
	}

	void StateMachineComponent::onStateExited(State& state)
	{
		// State exit was pending, handle, unsubscribe and transition
		enterNextStateNow();
	}

	void StateMachineComponent::executeMessage(const std::string& name)
	{
		if (!mCurrentState) return;

		Transition* transition = findMessageTransition(name);
		if (!transition) return;

        Logger::debug(*this, "Message executed: '%s'", name.c_str());

        executeTransition(transition);
	}


	nap::Transition* StateMachineComponent::findMessageTransition(const std::string& name)
	{
		for (auto trans : getTransitions(*mCurrentState)) {
			if (!trans->getTypeInfo().isKindOf<MessageTransition>()) continue;
			MessageTransition* actionTransition = static_cast<MessageTransition*>(trans);

			if (actionTransition->messageString.getValue() == name) {
				return actionTransition;
            }
		}
		return nullptr;
	}

	AutoTransition* StateMachineComponent::findAutoTransition(const State& fromState) {
        for (auto trans : getTransitions(fromState)) {
            if (!trans->getTypeInfo().isKindOf<AutoTransition>())
                continue;
            return static_cast<AutoTransition*>(trans);
        }
        return nullptr;
    }


	std::vector<Transition*> StateMachineComponent::getTransitions(const State& state)
	{
		std::vector<Transition*> transitions;
		for (auto transition : getTransitions()) {
			if (transition->getSrcState() == &state || transition->getSrcState() == &getAnyState())
				transitions.emplace_back(transition);
		}
		return transitions;
	}


	void StateMachineComponent::executeTransition(Transition* trans)
	{
		if (mNextState) {
			if (trans->isInterruptible()) {
				mCurrentState->finishTransition();
				Logger::info("Finishing transition: %s", ObjectPath(trans).toString().c_str());
			} else {
				return;
			}
		}
		enterStatePrepare(*trans->getDstState());
	}


	bool StateMachineComponent::handleEvent(Event& event)
	{
		if (event.getTypeInfo().isKindOf<Message>()) executeMessage(static_cast<Message*>(&event)->getMessage());
		return false;
	}



	void StateMachineComponent::update(const float& dt)
	{
		if (mCurrentState) {
			mCurrentState->update(dt);

			if (mNextState && mCurrentState->getCurrentMode() == Exited) enterNextStateNow();
		}
	}
}
