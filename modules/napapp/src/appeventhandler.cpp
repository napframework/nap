// Local Includes
#include "appeventhandler.h"

// External includes
#include <sdlinput.h>
#include <sdlwindow.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseAppEventHandler)
	RTTI_CONSTRUCTOR(nap::BaseApp&)
RTTI_END_CLASS	

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GUIAppEventHandler)
	RTTI_CONSTRUCTOR(nap::App&)
RTTI_END_CLASS


namespace nap
{
	BaseAppEventHandler::BaseAppEventHandler(BaseApp& app) : mApp(app)
	{ }


	AppEventHandler::AppEventHandler(App& app) : BaseAppEventHandler(app)
	{ }


	GUIAppEventHandler::GUIAppEventHandler(App& app) : BaseAppEventHandler(app)
	{ }


	void AppEventHandler::process()
	{
		opengl::Event event;
		while (opengl::pollEvent(event))
		{
			// Stop if the event tells us to quit
			if (event.type == SDL_QUIT)
			{
				mApp.quit(0);
				break;
			}

			// Check if we are dealing with an input event (mouse / keyboard)
			if (nap::isInputEvent(event))
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);

				// Register our input event with the appRunner
				getApp<App>().inputMessageReceived(std::move(input_event));
			}

			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(event);
				if (window_event != nullptr)
				{
					getApp<App>().windowMessageReceived(std::move(window_event));
				}
			}
		}
	}


	void GUIAppEventHandler::process()
	{
		// Poll for events
		opengl::Event event;
		while (opengl::pollEvent(event))
		{
			// Stop if the event tells us to quit
			if (event.type == SDL_QUIT)
			{
				mApp.quit(0);
				break;
			}

			// Get the interface
			ImGuiIO& io = ImGui::GetIO();
			
			// Process event for imgui
			ImGui_ImplSdlGL3_ProcessEvent(&event);

			// Forward if we're not capturing mouse and it's a pointer event
			if (nap::isPointerEvent(event) && !io.WantCaptureMouse)
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				getApp<App>().inputMessageReceived(std::move(input_event));
				continue;
			}

			// Forward if we're not capturing keyboard and it's a key event
			if (nap::isKeyEvent(event) && !io.WantCaptureKeyboard)
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				getApp<App>().inputMessageReceived(std::move(input_event));
				continue;
			}

			// Always forward window events
			if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(event);
				if (window_event != nullptr)
				{
					getApp<App>().windowMessageReceived(std::move(window_event));
				}
			}
		}
	}
}