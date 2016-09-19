#pragma once
#include "animatedattributecontroller.h"
#include "attributeanimator.h"
#include "attributecontroller.h"
#include "MessageCtrl.h"
#include "statecontroller.h"

namespace nap
{
	class StateMachineComponent;


	class State : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		State() : AttributeObject() {}

		void update(const float& dt);


		// Add controller that will control the value of provided attribute
		AttrCtrl& addAttrCtrl(AttributeBase& targetAttribute,
													const StateMode& trigger = Entering);

        template<typename T>
        AttrCtrl& addAttrCtrl(Attribute<T>* targetAttribute, T targetValue, const StateMode& trigger = Entering) {
            return addAttrCtrl(*targetAttribute, targetValue, trigger);
        }

		// Add a regular attribute controller that will immediately set the value upon activation
		template <typename T>
		AttrCtrl& addAttrCtrl(Attribute<T>& targetAttribute, T targetvalue,
													const StateMode& trigger = Entering)
		{
			assert(targetAttribute.getValueType() == RTTI_OF(T));

			AttrCtrl& ctrl = addAttrCtrl(targetAttribute, trigger);
			Attribute<T>* ctrlAttrib = static_cast<Attribute<T>*>(ctrl.getControlAttribute());
			ctrlAttrib->setValue(targetvalue);

			return ctrl;
		}

		// Send a message upon activation
		MessageCtrl& addMessageCtrl(const std::string& action, DispatchMethod methods,
												const StateMode& trigger = Entering, Object* senderObject = nullptr);

		// Start animating a value upon activation
		template <typename T>
		AnimAttrCtrl& addAnimCtrl(Attribute<T>& targetAttrib, AttributeAnimator<T>* animator,
														   const StateMode& trigger = Entering)
		{
			assert(animator);
			auto& controller = addCtrl<AnimAttrCtrl>();
			controller.setTrigger(trigger);
			controller.setName(targetAttrib.getName() + "_Controller");
			controller.setTargetAttribute(targetAttrib);
			controller.setAnimator(std::unique_ptr<AttributeAnimatorBase>(animator));
			return controller;
		}

		// 		template<typename T>
		// 		T& addController(const std::string& name, const StateMode& trigger) {
		// 			return *static_cast<T*>(&addController(name, RTTI_OF(T), trigger));
		// 		}

		template <typename T, typename... Args>
		T& addCtrl(Args&&... args)
		{
			RTTI::TypeInfo type = RTTI_OF(T);
			assert(type.isKindOf<StateCtrl>());
			return *static_cast<T*>(&addChild(std::make_unique<T>(std::forward<Args>(args)...)));
		}

		StateCtrl& addCtrl(const std::string& name, const RTTI::TypeInfo& type, const StateMode& trigger)
		{
			assert(type.isKindOf<StateCtrl>());
			StateCtrl& ctrl = *static_cast<StateCtrl*>(&addChild(name, type));
			ctrl.setTrigger(trigger);
			return ctrl;
		}

		const StateMode& getCurrentMode() const { return mCurrentMode; }

		StateMachineComponent& getStateMachine();

		void enter();
		void exit();
		void finishTransition();

		Signal<State&> entering;
		Signal<State&> entered;
		Signal<State&> exiting;
		Signal<State&> exited;

		// TODO: Remove
		Attribute<bool> blockingTransition = { this, "blockingTransition", true };

	private:
		void onTransitionFinished();

		// Store current mode and emit signal
		void setCurrentMode(const StateMode& mode);

		StateMode mCurrentMode = Exited;
		std::vector<StateCtrl*> getControllers(const StateMode& trigger);
	};
}

RTTI_DECLARE(nap::State)
