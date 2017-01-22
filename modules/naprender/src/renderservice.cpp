// Local Includes
#include "renderservice.h"
#include "meshcomponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"
#include "openglrenderer.h"
#include "transformcomponent.h"

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
		core.registerType(*this, RTTI_OF(TransformComponent));
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
		
		// Set window on window component
		window.mWindow.reset(new_window);

		// Initialize Glew
		if (state == State::Uninitialized)
		{
			if (!mRenderer->postInit())
			{
				state = State::SystemError;
				nap::Logger::fatal(*this, "unable to finalize render initialization process");
				return;
			}

			// Start timer
			mTimer.start();

			// Store previous frame time
			mFrameTimeStamp = mTimer.getStartTime();

			// Set start time for fps counter
			mFpsTime   = mTimer.getElapsedTime();
		}

		state = State::Initialized;
	}


	// Updates internal fps counter
	void RenderService::updateFpsCounter(double deltaTime)
	{
		mFpsTime += deltaTime;
		if (mFpsTime < 0.1)
		{
			return;
		}

		mFps = (float)((double)mFrames / (mFpsTime));
		mFpsTime = 0.0;
		mFrames = 0;
	}


	// Finds all top level transforms
	void RenderService::getTopLevelTransforms(Entity* entity, std::vector<TransformComponent*>& xforms)
	{
		// Get xform on current entity
		nap::TransformComponent* xform_comp = entity->getComponent<TransformComponent>();

		// If we found one, add it
		if (xform_comp != nullptr)
		{
			xforms.emplace_back(xform_comp);
			return;
		}

		// If not try it's children
		for (auto& child : entity->getEntities())
		{
			getTopLevelTransforms(child, xforms);
		}
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Emits the draw call
	void RenderService::render()
	{
		if (state == State::Uninitialized)
		{
			nap::Logger::fatal(*this, "unable to execute render call, service is not initialized");
			return;
		}

		if (state > State::Initialized)
		{
			nap::Logger::fatal(*this, "unable to execute render call, internal error occurred: %d", static_cast<int>(state));
			return;
		}

		// Get all window components
		std::vector<RenderWindowComponent*> windows;
		getObjects<RenderWindowComponent>(windows);
		
		// Trigger update
		for (auto& window : windows)
		{
			window->activate.trigger();
			window->update.trigger();
		}

		// Collect all transform changes and push
		updateTransforms();

		// Trigger render call
		for (auto& window : windows)
		{
			window->activate.trigger();
			window->draw.trigger();
			window->swap();
		}

		// Increment number of rendered frames
		mFrames++;

		// Store amount of time in nanoseconds it took to compute frame
		TimePoint current_time = getCurrentTime();
		mDeltaTime = current_time - mFrameTimeStamp;

		// Update timestamp to be current time
		mFrameTimeStamp = current_time;

		// Update fps
		updateFpsCounter(getDeltaTime());
	}


	// Updates all transform components
	void RenderService::updateTransforms()
	{
		std::vector<TransformComponent*> top_xforms;
		getTopLevelTransforms(&(getCore().getRoot()), top_xforms);
		for (auto& xform : top_xforms)
			xform->update();
	}


	// Renders all available objects
	void RenderService::renderObjects()
	{
		// Get all render components
		std::vector<nap::RenderableComponent*> render_comps;
		getObjects<nap::RenderableComponent>(render_comps);

		// Draw
		for (auto& comp : render_comps)
			comp->draw();
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


	// return number of elapsed ticks
	uint32 RenderService::getTicks() const
	{
		return mTimer.getTicks();
	}


	// Return elapsed time
	double RenderService::getElapsedTime() const
	{
		return mTimer.getElapsedTime();
	}


	// Return compute last frame compute time
	double RenderService::getDeltaTime() const
	{
		return std::chrono::duration<double>(mDeltaTime).count();
	}


	// Frame compute time in float
	float RenderService::getDeltaTimeFloat() const
	{
		return std::chrono::duration<float>(mDeltaTime).count();
	}


	float RenderService::getFps() const
	{
		return mFps;
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