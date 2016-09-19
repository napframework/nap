#pragma once

#include "state.h"
#include "statecontroller.h"
#include "transition.h"
#include <nap.h>
#include <nap/event.h>

namespace nap
{
	// State flow works as follows
	//
	// State_Entering -> State_Entered -/-> Transition_Execute -> State_Exiting -> State_Exited -> NextState_Entering
	// ...
	//
	class StateMachineComponent : public EventHandlerComponent
	{
		RTTI_ENABLE_DERIVED_FROM(EventHandlerComponent)
	public:
		StateMachineComponent();

		void initialize();

		// Frequent trigger, used figure out wherether any state controllers are still active
		void update(const float& dt);

		State& addState(const std::string& name);
		State* getState(const std::string& name);

		// A wildcard state enabling a transition to always execute, regardless of the current state
		State& getAnyState() const { return *mAnyState; }
        State& getCurrentState() const { return *mCurrentState; }
		State& getHistoryState() const { return *mHistoryStateStub; }

		// Find a message transition and execute it if possible
		void executeMessage(const std::string& name);

        // Add a transition that will automatically transfer to @toState, when @fromState has entered
        Transition* addAutoTransition(State& fromState, State& toState);

		// Add a transition that will trigger when a specific message has been received by the fsm
		// @fromState: The possible state to transition from
		// @toState: The state to transition to when triggered
		Transition* addMessageTransition(State& fromState, State& toState, const std::string& message);

		// Add a transition that will trigger when a specific message has been received by the fsm
		// @toState: The state to transition to when triggered
		Transition* addMessageTransition(State& toState, const std::string& message);

		// The state on which this StateMachine will start, must be set
		Attribute<std::string> initialState = {this, "InitialState"};

		// Trigger state thresholds
		Signal<State&> stateEntering;
		Signal<State&> stateEntered;
		Signal<State&> stateExiting;
		Signal<State&> stateExited;

		// TODO: Make private
		void executeTransition(Transition* trans);

		bool handleEvent(Event& event) override;

	private:
		// Start entering a state
		void enterStatePrepare(State& state);

		// Enter the next state immediately
		void enterNextStateNow();

		// Start exiting state
		void exitState(State& state) { state.exit(); }

		// Will be invoked when state exit has finished
		void onStateExited(State& state);

		Transition* findMessageTransition(const std::string& name);

        AutoTransition* findAutoTransition(const State& fromState);

		// Retrieve all transitions in this state machine
		std::vector<Transition*> getTransitions() { return getChildrenOfType<Transition>(); }

		// Retrieve all transitions originating from the specified State
		std::vector<Transition*> getTransitions(const State& state);

		// Retrieve all states in this statemachine
		std::vector<State*> getStates() { return getChildrenOfType<State>(); }

		// Set the state on which this statemachine should start
		void setInitialState(State& state) { initialState.setValue(state.getName()); }

		// Keep track of the next state while exiting the current state
		State* mNextState = nullptr;

		// Keep track of the current state
		State* mCurrentState = nullptr;

		// A stub state used for when a transition can be triggered from any state
		State* mAnyState = nullptr;

		State* mHistoryStateStub = nullptr;
        State* mHistoryState = nullptr;
	};
}

RTTI_DECLARE(nap::StateMachineComponent)
