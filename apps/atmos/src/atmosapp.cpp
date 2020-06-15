#include "atmosapp.h"

// Nap includes
#include <nap/core.h>
#include <parameterservice.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <imguiutils.h>

#include "applysensorcomponent.h"

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AtmosApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool AtmosApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("atmos.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
		mVideoTarget  = mResourceManager->findObject<nap::RenderTarget>("VideoRenderTarget");

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
	
		mCameraEntity = scene->findEntity("CameraEntity");
		mWorldEntity = scene->findEntity("WorldEntity");
		mScanEntity = scene->findEntity("ScanEntity");
		mVideoEntity = scene->findEntity("VideoPlaneEntity");
		mVideoCameraEntity = scene->findEntity("VideoCameraEntity");
		mLineEntity = scene->findEntity("LineEntity");
		mSensorEntity = scene->findEntity("SensorEntity");

		//holds the preset selector component (better name for this entity?)
		mUniverseEntity = scene->findEntity("UniverseEntity"); 

		// Get yocto sensors
		mProximitySensor = mResourceManager->findObject("ProximitySensor");

		// Get apply range sensor component instance
		auto rangeSensorComponent = mSensorEntity->findComponentByID("ApplyRangeSensorComponent");
		assert(rangeSensorComponent != nullptr); // component not found
		assert(rangeSensorComponent->get_type() == RTTI_OF(ApplySensorComponentInstance)); // type mismatch
		if (error.check(rangeSensorComponent == nullptr || rangeSensorComponent->get_type() != RTTI_OF(ApplySensorComponentInstance), "Range Sensor Component not found or type mismatch!"))
		{
			return false;
		}
		mApplySensorComponent = static_cast<ApplySensorComponent*>(static_cast<ApplySensorComponentInstance*>(rangeSensorComponent)->getComponent());

		// Create gui
		mGui = std::make_unique<AtmosGui>(*this);
		mGui->init();

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 */
	void AtmosApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get(), mSensorEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update GUI
		mGui->update();
	}

	
	/**
	 * Rendering
	 */
	void AtmosApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Render video into back-buffer
		{
			mRenderService->clearRenderTarget(mVideoTarget->getTarget());
			mRenderService->setPolygonMode(opengl::EPolygonMode::Fill);
			std::vector<nap::RenderableComponentInstance*> render_comps;
			mVideoEntity->getComponentsOfType<RenderableComponentInstance>(render_comps);
			nap::OrthoCameraComponentInstance& video_cam = mVideoCameraEntity->getComponent<nap::OrthoCameraComponentInstance>();
			mRenderService->renderObjects(mVideoTarget->getTarget(), video_cam, render_comps);
		}

		// Render meshes etc. to main window
		{
			// Clear back-buffer
			mRenderWindow->getBackbuffer().setClearColor(mGui->getBackgroundColor());
			mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

			// Comps to render
			std::vector<nap::RenderableComponentInstance*> render_comps;

			// Set draw mode (fill, lines, polygon)
			mRenderService->setPolygonMode(mGui->getRenderMode());

			// Render Scan
			mScanEntity->getComponentsOfType<nap::RenderableComponentInstance>(render_comps);
			nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();
			mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, render_comps);

			if (mGui->renderPath())
			{
				// Render line
				render_comps.clear();
				mLineEntity->getComponentsOfType<nap::RenderableComponentInstance>(render_comps);
				mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, render_comps);
			}

			// Render gui to window
			mGuiService->draw();

			// Swap screen buffers
			mRenderWindow->swap();
		}
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void AtmosApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void AtmosApp::inputMessageReceived(InputEventPtr inputEvent)
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

			//demo functionality for the preset selector: 
//			if (press_event->mKey == nap::EKeyCode::KEY_1)
//			{
//				nap::SwitchPresetComponentInstance& presetSelector = mUniverseEntity->getComponent <nap::SwitchPresetComponentInstance>();
//				presetSelector.selectPresetByIndex(0);
//			}

//			if (press_event->mKey == nap::EKeyCode::KEY_2)
//			{
//				nap::SwitchPresetComponentInstance& presetSelector = mUniverseEntity->getComponent <nap::SwitchPresetComponentInstance>();
//				presetSelector.selectPresetByIndex(1);
//			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	int AtmosApp::shutdown()
	{
		mGui.reset();
		return 0;
	}
}