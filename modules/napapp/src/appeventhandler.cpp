// Local Includes
#include "appeventhandler.h"
#include "sdlinputservice.h"

// External includes
#include <sdleventconverter.h>
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


	void AppEventHandler::start()
	{
		SDLInputService* input_service = mApp.getCore().getService<nap::SDLInputService>();
		assert(input_service);
		mEventConverter = std::make_unique<SDLEventConverter>(*input_service);
	}


	GUIAppEventHandler::GUIAppEventHandler(App& app) : BaseAppEventHandler(app)
	{ }


	void GUIAppEventHandler::start()
	{
		SDLInputService* input_service = mApp.getCore().getService<nap::SDLInputService>();
		assert(input_service);
		mEventConverter = std::make_unique<SDLEventConverter>(*input_service);

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SDLK_a;
		io.KeyMap[ImGuiKey_C] = SDLK_c;
		io.KeyMap[ImGuiKey_V] = SDLK_v;
		io.KeyMap[ImGuiKey_X] = SDLK_x;
		io.KeyMap[ImGuiKey_Y] = SDLK_y;
		io.KeyMap[ImGuiKey_Z] = SDLK_z;
	}


	void AppEventHandler::process()
	{
		opengl::Event event;
		while (opengl::pollEvent(event))
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
				nap::WindowEventPtr window_event = mEventConverter->translateWindowEvent(event);
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


	void AppEventHandler::shutdown()
	{
		mEventConverter.reset(nullptr);
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
			if (mEventConverter->isMouseEvent(event))
			{
				if (!io.WantCaptureMouse)
				{
					nap::InputEventPtr input_event = mEventConverter->translateMouseEvent(event);
					if (input_event != nullptr)
					{
						getApp<App>().inputMessageReceived(std::move(input_event));
					}
				}
			}

			// Forward if we're not capturing keyboard and it's a key event
			else if (mEventConverter->isKeyEvent(event))
			{
				if (!io.WantCaptureKeyboard)
				{
					nap::InputEventPtr input_event = mEventConverter->translateKeyEvent(event);
					if (input_event != nullptr)
					{
						getApp<App>().inputMessageReceived(std::move(input_event));
					}
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
				nap::WindowEventPtr window_event = mEventConverter->translateWindowEvent(event);
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


	void GUIAppEventHandler::shutdown()
	{
		mEventConverter.reset(nullptr);
	}

}