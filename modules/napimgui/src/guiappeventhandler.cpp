/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "guiappeventhandler.h"
#include "imguiservice.h"

// External includes
#include <sdlinputservice.h>
#include <SDL_hints.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GUIAppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

namespace nap
{
	GUIAppEventHandler::GUIAppEventHandler(App& app) : AppEventHandler(app)
	{
		if (!setTouchGeneratesMouseEvents(true))
		{
			nap::Logger::warn("Unable to control if touch input generates mouse events");
		}
	}


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
			// Forward if we're not capturing the mouse in the GUI and it's a pointer event
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

			// Forward if we're not capturing the keyboard in the GUI and it's a key event
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

			// Always forward touch events
			else if (mEventConverter->isTouchEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateTouchEvent(event);
				if (input_event == nullptr)
					continue;

				// Forward touch as mouse input to GUI if touch input doesn't generate mouse events.
				assert(input_event->get_type().is_derived_from(RTTI_OF(WindowInputEvent)));
				ImGuiContext* ctx = !mTouchGeneratesMouseEvents ?
					mGuiService->processInputEvent(*input_event) :
					mGuiService->findContext(static_cast<WindowInputEvent*>(input_event.get())->mWindow);

				// Forward touch input to running app if gui isn't capturing this 'touch' event
				if (ctx != nullptr && !mGuiService->isCapturingMouse(ctx))
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


	bool GUIAppEventHandler::setTouchGeneratesMouseEvents(bool value)
	{
		if (SDL_SetHintWithPriority(SDL_HINT_TOUCH_MOUSE_EVENTS, value ? "1" : "0",
			SDL_HintPriority::SDL_HINT_OVERRIDE) > 0)
		{
			mTouchGeneratesMouseEvents = value;
			return true;
		}
		return false;
	}
}
