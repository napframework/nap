// Local Includes
#include "renderservice.h"
#include "meshcomponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"
#include "openglrenderer.h"

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
			nap::Logger::fatal(*this, "unable to create new window, previous error occurred");
			return;
		}

		// Make sure we have a renderer
		if (mRenderer == nullptr)
		{
			nap::Logger::fatal(*this, "unable to create new window, no associated renderer");
			state = State::SystemError;
			return;
		}

		// Initialize video render 
		if (state == State::Uninitialized)
		{
			if (!mRenderer->preInit())
			{
				nap::Logger::fatal(*this, "unable to initialize renderer");
				state = State::SystemError;
				return;
			}
		}

		// Get settings
		const nap::RenderWindowSettings& window_settings = window.getConstructionSettings();
		nap::RenderWindow* new_window = mRenderer->createRenderWindow(window_settings);
		if (new_window == nullptr)
		{
			nap::Logger::fatal(*this, "unable to create render window and context");
			state = State::WindowError;
			return;
		}
		
		// Set window
		window.mWindow.reset(static_cast<OpenGLRenderWindow*>(new_window)->getContainer());

		// Initialize Glew
		if (state == State::Uninitialized)
		{
			if (!mRenderer->postInit())
			{
				state = State::SystemError;
				nap::Logger::fatal(*this, "unable to finalize render initialzation process");
				return;
			}
		}
		state = State::Initialized;
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
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


	// Set the currently active renderer
	void RenderService::setRenderer(const RTTI::TypeInfo& renderer)
	{
		if (!renderer.isKindOf(RTTI_OF(nap::Renderer)))
		{
			nap::Logger::warn(*this, "unable to add: %s as renderer, object not of type: %s", renderer.getName().c_str(), RTTI_OF(nap::Renderer).getName().c_str());
			return;
		}

		// Shut down existing renderer
		shutdown();

		// Set state
		state = State::Uninitialized;

		// Create new renderer
		nap::Renderer* new_renderer = static_cast<nap::Renderer*>(renderer.createInstance());
		mRenderer.reset(new_renderer);
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
		if (state == State::Initialized)
		{
			assert(mRenderer != nullptr);
			mRenderer->shutdown();
		}
		state = State::Uninitialized;
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