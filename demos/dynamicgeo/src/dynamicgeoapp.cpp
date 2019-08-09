#include "dynamicgeoapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <nap/datetime.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DynamicGeoApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	* Initialize all the resources and store the objects we need later on
	*/
	bool DynamicGeoApp::init(utility::ErrorState& error)
	{		
		// Create render service
		mRenderService	= getCore().getService<RenderService>();		
		mInputService	= getCore().getService<InputService>();
		mSceneService	= getCore().getService<SceneService>();
		mGuiService		= getCore().getService<IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("objects.json", error))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", error.toString().c_str());
			return false;                
		}
		
		ObjectPtr<Scene> scene		= mResourceManager->findObject<Scene>("Scene");
		mRenderWindow				= mResourceManager->findObject<RenderWindow>("Window0");
		mCameraEntity				= scene->findEntity("CameraEntity");
		mDefaultInputRouter			= scene->findEntity("DefaultInputRouterEntity");
		return true;
	}
	
	
	/**
	 * Forward all received input events to the input router. 
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our first person camera.
	 *
	 * We also set up our gui that is drawn at a later stage.
	 */
	void DynamicGeoApp::update(double deltaTime)
	{
		DefaultInputRouter& input_router = mDefaultInputRouter->getComponent<DefaultInputRouterComponentInstance>().mInputRouter;
		{
			// Update input for first window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntity.get());

			Window* window = mRenderWindow.get();
			mInputService->processWindowEvents(*window, input_router, entities);
		}

		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "wasd keys to move, mouse + left mouse button to look");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();
	}
	
	
	/**
	 * Render all objects to screen at once
	 * In this case that's only the particle mesh
	 */
	void DynamicGeoApp::render()
	{
		// Get rid of unnecessary resources
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear window back-buffer
		opengl::RenderTarget& backbuffer = mRenderWindow->getWindow()->getBackbuffer();
		mRenderService->clearRenderTarget(backbuffer);

		PerspCameraComponentInstance& frame_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
		mRenderService->renderObjects(backbuffer, frame_cam);

		// Render GUI elements
		mGuiService->draw();

		// Swap back buffer
		mRenderWindow->swap();
	}
	

	/**
	* Occurs when the event handler receives a window message.
	* You generally give it to the render service which in turn forwards it to the right internal window.
	* On the next update the render service automatically processes all window events.
	* If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	*/
	void DynamicGeoApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	/**
	* Called by the app loop. It's best to forward messages to the input service for further processing later on
	* In this case we also check if we need to toggle full-screen or exit the running app
	*/
	void DynamicGeoApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// Escape the loop when esc is pressed
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit();
			}
			// Toggle fullscreen on 'f'
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}

		}
		mInputService->addEvent(std::move(inputEvent));
	}

	// Cleanup
	int DynamicGeoApp::shutdown() 
	{
		std::cout << "stopping..." << "\n";
		return 0;
	}
}
