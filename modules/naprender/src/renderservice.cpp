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
		if (inObject.getTypeInfo().isKindOf(RTTI_OF(RenderWindowComponent)) && !glewInitialized)
		{
			opengl::init();
			glewInitialized = true;
		}
	}


	// Initializes opengl related functionality
	void RenderService::init()
	{
		// Initialize video
		opengl::initVideo();
		opengl::Attributes attributes;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.dubbleBuffer = true;
		attrs.versionMinor = 2;
		attrs.versionMajor = 3;
		opengl::setAttributes(attrs);
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