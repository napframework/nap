#include "app.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::App)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	BaseApp::BaseApp(nap::Core& core) : mCore(core)
	{

	}


	void BaseApp::quit(int errorCode /*= 0*/)
	{
		mExitCode = errorCode;
		mQuit = true;
	}


	App::App(Core& core) : BaseApp(core)
	{

	}

}
