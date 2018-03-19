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
			// Check if we are dealing with an input event (mouse / keyboard)
			if (nap::isInputEvent(event))
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				if (input_event != nullptr)
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
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

			// Check if we need to quit the app from running
			// -1 signals a quit cancellation
			else if (event.type == SDL_QUIT)
			{
				if (getApp<App>().shutdownRequested())
				{
					getApp<App>().quit();
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
			// Get the interface
			ImGuiIO& io = ImGui::GetIO();
			
			// Process event for imgui
			ImGui_ImplSdlGL3_ProcessEvent(&event);

			// Forward if we're not capturing mouse and it's a pointer event
			if (nap::isMouseEvent(event) && !io.WantCaptureMouse)
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				if (input_event != nullptr)
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Forward if we're not capturing keyboard and it's a key event
			else if (nap::isKeyEvent(event) && !io.WantCaptureKeyboard)
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				if (input_event != nullptr)
				{
					getApp<App>().inputMessageReceived(std::move(input_event));
				}
			}

			// Always forward window events
			else if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(event);
				if (window_event != nullptr)
				{
					getApp<App>().windowMessageReceived(std::move(window_event));
				}
			}

			// Stop if the event tells us to quit
			else if (event.type == SDL_QUIT)
			{
				if (getApp<App>().shutdownRequested())
				{
					getApp<App>().quit();
				}
			}
		}
	}
}