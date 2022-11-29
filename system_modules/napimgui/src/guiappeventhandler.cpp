/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "guiappeventhandler.h"
#include "imguiservice.h"

// External includes
#include <sdlinputservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GUIAppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

namespace nap
{
	GUIAppEventHandler::GUIAppEventHandler(App& app) : AppEventHandler(app)
	{ }

	void GUIAppEventHandler::start()
	{
		SDLInputService* input_service = mApp.getCore().getService<nap::SDLInputService>();
		assert(input_service);
		mEventConverter = std::make_unique<SDLEventConverter>(*input_service);

		mGuiService = mApp.getCore().getService<IMGuiService>();
		assert(mGuiService != nullptr);
	}


	void GUIAppEventHandler::process()
	{
		// Poll for events
		SDL_Event event;
		bool quit = false;
		while (SDL_PollEvent(&event))
		{
			// Forward if we're not capturing mouse and it's a pointer event
			if (mEventConverter->isMouseEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateMouseEvent(event);
				if (input_event == nullptr)
					continue;

				ImGuiContext* ctx = mGuiService->processInputEvent(*input_event);
				if (ctx != nullptr && !mGuiService->isCapturingMouse(ctx))
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Forward if we're not capturing keyboard and it's a key event
			else if (mEventConverter->isKeyEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateKeyEvent(event);
				if (input_event == nullptr)
					continue;

				ImGuiContext* ctx = mGuiService->processInputEvent(*input_event);
				if (ctx != nullptr && !mGuiService->isCapturingKeyboard(ctx))
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Always forward controller events
			else if (mEventConverter->isControllerEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateControllerEvent(event);
				if (input_event != nullptr)
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Always forward window events
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

			// Stop if the event tells us to quit
			else if (event.type == SDL_QUIT && getApp<App>().shutdownRequested())
			{
				getApp<App>().quit();
			}
		}
	}


	void GUIAppEventHandler::shutdown()
	{
		mEventConverter.reset(nullptr);
		mGuiService = nullptr;
	}
}
