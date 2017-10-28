#include "artnetapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtnetApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool ArtnetApp::init(utility::ErrorState& error)
	{
		// Create all services
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		mArtnetService = getCore().getService<ArtNetService>();
		
		// Load scene
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("data/artnet/artnet.json", error))
			return false;
		
		// Get important entities
		mCameraEntity = mResourceManager->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		// Store all render windows
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));
		
		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void ArtnetApp::update(double deltaTime)
	{		
		// Update and send our test data over ArtNET
		float sine = sin(mRenderService->getCore().getElapsedTime() * (M_PI * 2));
		
		std::vector<float> channel_data;
		channel_data.resize(512);
		
		for (int index = 0; index < channel_data.size(); ++index)
		{
			float offset = (sine + 1.0f) / 2.0f;
			channel_data[index] = std::min((float)index / (float)channel_data.size() + offset, 1.0f);
		}
		
		ObjectPtr<ArtNetController> universe_0 = mResourceManager->findObject("Universe0");
		universe_0->send(channel_data);
		
		ObjectPtr<ArtNetController> universe_1 = mResourceManager->findObject("Universe1");
		std::reverse(channel_data.begin(), channel_data.end());
		universe_1->send(channel_data);
	}
	
	
	// Called when the window is going to render
	void ArtnetApp::render()
	{
		mRenderService->destroyGLContextResources(mRenderWindows);
		
		// Activate current window for drawing
		mRenderWindows[0]->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(mRenderWindows[0]->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		
		// Swap backbuffer
		mRenderWindows[0]->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void ArtnetApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void ArtnetApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void ArtnetApp::inputMessageReceived(InputEventPtr inputEvent) 
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit(0);
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	void ArtnetApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void ArtnetApp::shutdown() 
	{
	}
}
