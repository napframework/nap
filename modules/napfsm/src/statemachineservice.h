#include <nap.h>

#include "statemachinecomponent.h"

namespace nap {

    class StateMachineService : public Service
    {
		RTTI_ENABLE_DERIVED_FROM(Service)
		NAP_DECLARE_SERVICE()
	public:
		StateMachineService() {}

		void update(float dt) {
			std::vector<StateMachineComponent*> statemachines;
			getComponents<StateMachineComponent>(statemachines);
			for (StateMachineComponent* fsm : statemachines)
				fsm->update(dt);
		}


    };
}

RTTI_DECLARE(nap::StateMachineService)