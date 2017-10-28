// Local Includes
#include "appeventhandler.h"

// External includes
#include <sdlinput.h>
#include <sdlwindow.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseAppEventHandler)
	RTTI_CONSTRUCTOR(nap::BaseApp&)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

namespace nap
{
	BaseAppEventHandler::BaseAppEventHandler(BaseApp& app) : mApp(app)
	{ }


	AppEventHandler::AppEventHandler(App& app) : BaseAppEventHandler(app)
	{ }


	void AppEventHandler::process()
	{
		while (opengl::pollEvent(mEvent))
		{
			// Check if we are dealing with an input event (mouse / keyboard)
			if (nap::isInputEvent(mEvent))
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(mEvent);

				// Register our input event with the appRunner
				getApp<App>().inputMessageReceived(std::move(input_event));
			}

			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(mEvent))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(mEvent);
				if (window_event != nullptr)
				{
					getApp<App>().windowMessageReceived(std::move(window_event));
				}
			}
		}
	}

}