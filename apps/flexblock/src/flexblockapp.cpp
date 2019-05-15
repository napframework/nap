// Local Includes
#include "flexblockapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <transformcomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <imguiutils.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexblockApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool FlexblockApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("flexblock.json", error))
			return false;

		// Get the render window
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window0");

		// Find the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = scene->findEntity("CameraEntity");
		mWorldEntity = scene->findEntity("WorldEntity");
		mBlockEntity = scene->findEntity("BlockEntity");

		// Create gui
		mGui = std::make_unique<FlexblockGui>(*this);
		mGui->init();

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 */
	void FlexblockApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Upate GUI
		mGui->update();
	}

	
	/**
	 * Rendering
	 */
	void FlexblockApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Update camera position
		setCameraPosition();

		// Get the perspective camera
		PerspCameraComponentInstance& persp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), persp_cam);

		// Render gui to window
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void FlexblockApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void FlexblockApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}

			// If 'h' is pressed toggle gui visibility
			if (press_event->mKey == nap::EKeyCode::KEY_h)
			{
				mGui->toggleVisibility();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int FlexblockApp::shutdown()
	{
		mGui.reset();
		return 0;
	}


	void FlexblockApp::setCameraPosition()
	{
		// Get the perspective camera xform
		TransformComponentInstance& cam_xform = mCameraEntity->getComponent<TransformComponentInstance>();

		// Set the camera position in the shaders
		RenderableMeshComponentInstance& block_render_comp = mBlockEntity->getComponent<RenderableMeshComponentInstance>();
		UniformVec3& cam_input = block_render_comp.getMaterialInstance().getOrCreateUniform<UniformVec3>("inCameraPosition");
		cam_input.setValue(math::extractPosition(cam_xform.getGlobalTransform()));
	}

}