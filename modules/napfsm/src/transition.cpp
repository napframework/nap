#include "transition.h"
#include "statemachinecomponent.h"

RTTI_DEFINE(nap::Transition)
RTTI_DEFINE(nap::MessageTransition)
RTTI_DEFINE(nap::AutoTransition)

namespace nap {

    StateMachineComponent &Transition::getStateMachine() {
        return *static_cast<StateMachineComponent *>(getParentObject());
    }

    bool Transition::isInterruptible() const { return mIsInterruptible; }




}