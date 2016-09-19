#include "statemachineservice.h"

RTTI_DEFINE(nap::StateMachineService);

namespace nap 
{
	void StateMachineService::sRegisterTypes(nap::Core& inCore, const nap::Service& inService)
	{
		inCore.registerType(inService, RTTI_OF(StateMachineComponent));
	}
}