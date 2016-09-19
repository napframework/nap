#pragma once
#include <nap.h>
#include <nap/eventdispatcher.h>
#include "statemode.h"

namespace nap
{
	class StateMachineComponent;
	class State;

	class StateCtrl : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		StateCtrl() {}
		StateCtrl(const std::string& name, const StateMode& mode) : mTrigger(mode) { setName(name); }
		StateCtrl(const StateMode& mode) : mTrigger(mode) {}



		virtual void activate() = 0;

        void setTrigger(const StateMode& trigger) { mTrigger = trigger; }
		const StateMode& getTrigger() { return mTrigger; }

        virtual void update(const float& dt) {}

		virtual bool isFinished() const { return true; }

		// Force finish the controller
		virtual void finish() = 0;
	private:
		StateMode mTrigger;
	};

}

RTTI_DECLARE_BASE(nap::StateCtrl)
