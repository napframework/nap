// Local Includes
#include "randomapp.h"

// External Includes
#include <mathutils.h>
#include <texture2d.h>
#include <rendertexture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>
#include <utility/stringutils.h>
#include <scene.h>
#include <planemesh.h>
#include <ctime>
#include <chrono>
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RandomApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	bool RandomApp::init(utility::ErrorState& error)
	{
		// Create services
		mRenderService = getCore().getService<nap::RenderService>();
		mInputService =  getCore().getService<nap::InputService>();
		mSceneService =  getCore().getService<nap::SceneService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager and load data
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("random.json", error))
			return false;    

		// Render window and texture target
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		
		// Callback when window event is received
		mRenderWindow->mWindowEvent.connect(std::bind(&RandomApp::handleWindowEvent, this, std::placeholders::_1));

		// All of our entities
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Set render states
		nap::RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;

		return true;
	}

	void RandomApp::update(double deltaTime)
	{
		nap::DefaultInputRouter input_router;

		// Update input for first window
		std::vector<nap::EntityInstance*> input_entities;
		input_entities.emplace_back(&(mScene->getRootEntity()));
		mInputService->processEvents(*mRenderWindow, input_router, input_entities);

		// Updat gui here...
	}


	void RandomApp::render()
	{
		// Clear all unnecessary gl resources lala
		mRenderService->destroyGLContextResources(std::vector<rtti::ObjectPtr<nap::RenderWindow>>({ mRenderWindow }));

		// Make render window active so we can draw to it
		mRenderWindow->makeActive();

		// Clear
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Draw gui
		mGuiService->draw();

		// Swap to front
		mRenderWindow->swap();
	}


	void RandomApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
	}


	int RandomApp::shutdown()
	{
		return 0;
	}


	void RandomApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void RandomApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit();
				return;
			}

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
				return;
			}
		}

		// Add event to input service for further processing
		mInputService->addEvent(std::move(inputEvent));
	}
}