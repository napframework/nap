/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <SDL_hints.h>
#include <nap/logger.h>

// Local Includes
#include "sdlappeventhandler.h"
#include "sdlinputservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SDLAppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

namespace nap
{
	SDLAppEventHandler::SDLAppEventHandler(App& app) : AppEventHandler(app)
	{
		if (!setTouchGeneratesMouseEvents(true))
		{
			nap::Logger::warn("Unable to control if touch input generates mouse events");
		}
	}


	void SDLAppEventHandler::start()
	{
		SDLInputService* input_service = mApp.getCore().getService<nap::SDLInputService>();
		assert(input_service);
		mEventConverter = std::make_unique<SDLEventConverter>(*input_service);
	}


	void SDLAppEventHandler::process()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) > 0)
		{
			// Check if we are dealing with an input event (mouse / keyboard)
			if (mEventConverter->isInputEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateInputEvent(event);
				if (input_event != nullptr)
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Check if we're dealing with a window event
			else if (mEventConverter->isWindowEvent(event))
			{
				// Quit when request to close
				if (event.window.event == SDL_WINDOWEVENT_CLOSE && getApp<App>().shutdownRequested())
				{
					getApp<App>().quit();
				}

				nap::WindowEventPtr window_event = mEventConverter->translateWindowEvent(event);
				if (window_event != nullptr)
				{
					getApp<App>().windowMessageReceived(std::move(window_event));
				}
			}

			// Check if we need to quit the app from running
			// -1 signals a quit cancellation
			else if (event.type == SDL_QUIT && getApp<App>().shutdownRequested())
			{
				getApp<App>().quit();
			}
		}
	}


	void SDLAppEventHandler::shutdown()
	{
		mEventConverter.reset(nullptr);
	}


	bool SDLAppEventHandler::setTouchGeneratesMouseEvents(bool value)
	{
		return SDL_SetHintWithPriority(SDL_HINT_TOUCH_MOUSE_EVENTS, value ? "1" : "0",
			SDL_HintPriority::SDL_HINT_OVERRIDE) > 0;
	}
}
