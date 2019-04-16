// Local Includes
#include "appeventhandler.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AppEventHandler)
	RTTI_CONSTRUCTOR(nap::BaseApp&)
RTTI_END_CLASS	

namespace nap
{
	AppEventHandler::AppEventHandler(BaseApp& app) : mApp(app)
	{ }
}