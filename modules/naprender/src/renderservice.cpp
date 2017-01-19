// Local Includes
#include "renderservice.h"
#include "meshcomponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"

// External Includes
#include <nap/core.h>

namespace nap
{
	// Register all types
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
		core.registerType(*this, RTTI_OF(MeshComponent));
		core.registerType(*this, RTTI_OF(RenderWindowComponent));
	}


	// Occurs when an object registers itself with the service
	void RenderService::objectRegistered(Object& inObject)
	{
		// If we have a render window component and glew hasn't been initialized
		// Initialize glew. Otherwise subsequent render calls will fail
		if (inObject.getTypeInfo().isKindOf(RTTI_OF(RenderWindowComponent)))
		{
			RenderWindowComponent& new_window = static_cast<RenderWindowComponent&>(inObject);
			createWindow(new_window);
		}
	}


	// Creates a new opengl window and assigns it to the component
	// TODO: Add Mutex
	void RenderService::createWindow(RenderWindowComponent& window)
	{
		// Make sure we don't procedeed when errors have been raised before
		if (state > State::Initialized)
		{
			nap::Logger::fatal("unable to create new window, previous error occurred");
			return;
		}

		// Initialize video render 
		if (state == State::Uninitialized)
		{
			if (!init())
			{
				nap::Logger::fatal(*this, "unable to initialize video sub system");
				state = State::SystemError;
				return;
			}
		}

		// Get settings
		nap::RenderWindowSettings* window_settings = window.constructionSettings.getTarget<RenderWindowSettings>();
		if (window_settings == nullptr)
		{
			nap::Logger::fatal(window, "unable to query window settings");
			state = State::WindowError;
			return;
		}

		// Convert settings to gl settings
		opengl::WindowSettings gl_window_settings;
		gl_window_settings.borderless = window_settings->borderless.getValue();
		gl_window_settings.resizable = window_settings->resizable.getValue();
		
		// If we need to share this context with another context
		// Resolve the link and get the window pointer
		if (window_settings->sharedWindow.isLinked())
		{
			RenderWindowComponent* share_window = window_settings->sharedWindow.getTarget<RenderWindowComponent>();
			assert(share_window != nullptr);
			assert(share_window->mWindow != nullptr);
			gl_window_settings.share = share_window->mWindow.get();
		}

		// Construct window using settings
		opengl::Window* new_window = opengl::createWindow(gl_window_settings);
		if (new_window == nullptr)
		{
			nap::Logger::fatal(window, "unable to create opengl window and context");
			state = State::WindowError;
			return;
		}
		
		// Set window
		window.mWindow.reset(new_window);

		// Initialize Glew
		if (state == State::Uninitialized)
		{
			if (!opengl::init())
			{
				nap::Logger::fatal(*this, "unable to initialize glew subsystem");
				state = State::GLError;
				return;
			}
		}
		state = State::Initialized;
	}


	// Initializes opengl related functionality
	bool RenderService::init()
	{
		// Initialize video
		if (!opengl::initVideo())
			return false;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.dubbleBuffer = true;
		attrs.versionMinor = 2;
		attrs.versionMajor = 3;
		opengl::setAttributes(attrs);

		return true;
	}


	// Emits the draw call
	void RenderService::render()
	{
		// Get all render components
		std::vector<nap::RenderableComponent*> render_comps;
		getObjects<nap::RenderableComponent>(render_comps);

		// Draw
		for (auto& comp : render_comps)
			comp->draw();

		// Trigger
		draw.trigger();
	}



	/*
	void RenderService::renderCall()
	{
		render();
		opengl::swap(*mWindow);
	}
	*/


	/*
	void RenderService::renderLoop()
	{
		camera.setFieldOfView(45.0f);
		camera.setAspectRatio((float)windowWidth, (float)windowHeight);

		// Get start
		auto t_start = std::chrono::high_resolution_clock::now();

		// Enable some gl specific stuff
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE_ARB);
		std::clock_t begin = std::clock();
		mIsRunning = true;
		while (mIsRunning) {
			std::cout << "Rendering..." << std::endl;
			SDL_Event event;
			if (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					mIsRunning = false;
				}

				if (event.type == SDL_KEYDOWN) {
					// TODO: Forward key presses to trigger attribute
				}
			}

			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					// Update gl viewport
					updateViewport(event.window.data1, event.window.data2);
				default:
					break;
				}
			}

			renderCall();

			SDL_Delay(1000);
		}

		if (mWindow) 
		{
            Logger::info("Destroying window");
			delete mWindow;
			mWindow = nullptr;
		}

		opengl::shutdown();
	}
	*/

} // Renderservice

RTTI_DEFINE(nap::RenderService)