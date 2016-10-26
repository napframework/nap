#pragma once

#include <nap.h>

namespace nap
{

	class ScriptInterpreter : public Object
	{
        RTTI_ENABLE_DERIVED_FROM(Object)
	public:
		virtual std::string evalScript(const std::string& cmd) = 0;
        void exit() {
             mIsExiting = true;
        }

        bool isExiting() const { return mIsExiting; }
	private:
        bool mIsExiting = false;
	};

}

RTTI_DECLARE_BASE(nap::ScriptInterpreter)
