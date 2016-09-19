
#include "state.h"
#include "statemachinecomponent.h"

RTTI_DEFINE(nap::State)

namespace nap
{


	AttrCtrl& State::addAttrCtrl(AttributeBase& targetAttribute, const StateMode& trigger)
	{
		return addCtrl<AttrCtrl>(targetAttribute, trigger);
	}

	MessageCtrl & State::addMessageCtrl(const std::string &action, DispatchMethod flags, const StateMode &trigger, Object *senderObject) {
		auto& ctrl = addCtrl<MessageCtrl>();
		ctrl.setTrigger(trigger);
		ctrl.setName("Message_" + action + "_Controller");
		ctrl.setMessage(action);
		ctrl.setDispatchFlags(flags);

        if (senderObject) {
            ctrl.setSenderObject(*senderObject);
        } else {
            ctrl.setSenderObject(getStateMachine());
        }

		return ctrl;
	}

	std::vector<StateCtrl*> State::getControllers(const StateMode& trigger)
	{
		std::vector<StateCtrl*> controllers;
		for (StateCtrl* ctrl : getChildrenOfType<StateCtrl>())
			if (ctrl->getTrigger() == trigger)
				controllers.push_back(ctrl);
		return controllers;
	}


	void State::enter()
	{
		setCurrentMode(Entering);
	}

	void State::exit()
	{
		setCurrentMode(Exiting);
	}



	nap::StateMachineComponent& State::getStateMachine()
	{
		return *(StateMachineComponent*)getParentObject();
	}

	void State::update(const float &dt) {
		// Only update controllers designated for the current mode
		// Then, only if all controllers are finished, handle
		if (getCurrentMode() == Entering || getCurrentMode() == Exiting) 
		{
			bool isFinished = true;
			for (auto ctrl : getControllers(mCurrentMode))
			{
				if (!ctrl->isFinished()) {
					ctrl->update(dt);
					isFinished = false;
				}
			}

			if (isFinished) 
				onTransitionFinished();
		}
    }

	void State::finishTransition() {
		for (auto ctrl : getControllers(getCurrentMode())) {
			if (!ctrl->isFinished())
				ctrl->finish();
		}
	}

	void State::setCurrentMode(const StateMode& mode)
	{
		std::string statePath = ObjectPath(this).toString();

		mCurrentMode = mode;
		switch (mode) {
		case Entering:
			entering(*this);
			Logger::debug(*this, "Entering...");
			break;

		case Entered:
			entered(*this);
			Logger::debug(*this, "Entered.");
			break;

		case Exiting:
			exiting(*this);
			Logger::debug(*this, "Exiting...");
			break;

		case Exited:
			exited(*this);
			Logger::debug(*this, "Exited.");
			break;

		default:
			assert(false);
		}

        for (auto& ctrl : getControllers(mode)) {
            ctrl->activate();
            ctrl->update(0);
        }
    }

	void State::onTransitionFinished() {
		if (getCurrentMode() == Entering) {
			setCurrentMode(Entered);
		} else if (getCurrentMode() == Exiting) {
			setCurrentMode(Exited);
		} else {
			assert(false);
		}
	}


}