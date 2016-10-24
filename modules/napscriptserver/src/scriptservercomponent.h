#pragma once

#include <nap.h>
#include "scriptinterpreter.h"
#include "jsonrpcinterpreter.h"

namespace nap
{


    class ScriptServerComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		ScriptServerComponent();

		void run();
		Attribute<int> port = {this, "port", 8888};

	private:
		ScriptInterpreter* mInterpreter;

	};
}

RTTI_DECLARE(nap::ScriptServerComponent)