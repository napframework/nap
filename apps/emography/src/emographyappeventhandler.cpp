// Local includes
#include "emographyappeventhandler.h"

// External includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyAppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

namespace nap
{
	EmographyAppEventHandler::EmographyAppEventHandler(App& app) : AppEventHandler(app)
	{ }


	void EmographyAppEventHandler::start()
	{
		nap::Logger::info("Starting emography application event loop");
		return;
	}


	void EmographyAppEventHandler::process()
	{
		// Poll for events
		while (false)
		{

		}
	}


	void EmographyAppEventHandler::shutdown()
	{

	}
}