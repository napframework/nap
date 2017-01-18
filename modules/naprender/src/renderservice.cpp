// Local Includes
#include "renderservice.h"
#include "meshcomponent.h"
#include "rendercomponent.h"

// External Includes
#include <nap/core.h>

namespace nap
{
	// Register all types
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
		core.registerType(*this, RTTI_OF(MeshComponent));
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


	void RenderService::renderCall()
	{
		render();
		opengl::swap(*mWindow);
	}


	opengl::Window* RenderService::getWindow()
	{
		if (mWindow)
			return mWindow;

		initOpenGL();
		// Not initialized, create
		mThread = std::make_unique<std::thread>(std::bind(&RenderService::renderLoop, this));
		return mWindow;
	}


	bool RenderService::initOpenGL()
	{
		Logger::debug("Initializing OpenGL");
		// Initialize OpenGL
		if (!opengl::initVideo())
			return false;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.dubbleBuffer = true;
		attrs.versionMinor = 2;
		attrs.versionMajor = 3;
		opengl::setAttributes(attrs);

		// Create Window
		opengl::WindowSettings window_settings;
		window_settings.width = windowWidth;
		window_settings.height = windowHeight;
		window_settings.borderless = false;
		window_settings.resizable = true;
		window_settings.title = "RenderWindow";

		// Print error if window could not be created
		mWindow = opengl::createWindow(window_settings);
		if (mWindow == nullptr)
			return false;

		// Initialize glew
		opengl::init();

		// Enable multi sampling
		glEnable(GL_MULTISAMPLE);

		int Buffers(1), Samples(4);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &Buffers);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &Samples);

		Logger::debug("OpenGL initialized");

		return true;
	}


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


	void RenderService::destroyWindow(opengl::Window* window)
	{
		assert(window == mWindow);
		mIsRunning = false;
		mThread->join();
	}


	void RenderService::updateViewport(int width, int height)
	{
		glViewport(0, 0, width, height);
		camera.setAspectRatio((float)width, (float)height);
	}

} // Renderservice

RTTI_DEFINE(nap::RenderService)