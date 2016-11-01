#pragma once

#include "jsonrpcinterpreter.h"
#include "scriptinterpreter.h"
#include <nap.h>

namespace nap
{

	class ScriptServerComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		ScriptServerComponent() { running.valueChangedSignal.connect([&](const bool& running) {
        onRunningChanged(running);
            }); }

		virtual void run() = 0;

		Attribute<int> port = {this, "port", 8888};
		Attribute<bool> running = {this, "running", false};

	protected:
		ScriptInterpreter* mInterpreter;

	private:
//		Slot<const bool&> onRunningChangedSlot = { &onRunningChanged };

		void onRunningChanged(const bool& running)
		{
			if (running) {
				run();
			}
		}
	};

	class JSONRPCServerComponent : public ScriptServerComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ScriptServerComponent)
	public:
		JSONRPCServerComponent() : ScriptServerComponent()
		{
			mInterpreter = &addChild<JSONRPCInterpreter>("ScriptInterpreter");
		}

        void run() override;
	};


}

RTTI_DECLARE_BASE(nap::ScriptServerComponent)
RTTI_DECLARE(nap::JSONRPCServerComponent)
RTTI_DECLARE(nap::PythonServerComponent)