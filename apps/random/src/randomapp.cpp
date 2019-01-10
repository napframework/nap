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
#include <uniforms.h>
#include <orthocameracomponent.h>
#include <imguiutils.h>
#include <rendercombinationcomponent.h>
#include <lightingmodecomponent.h>

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
		mRenderWindow->setWidth(1920);
		mRenderWindow->setHeight(1080);
		
		// Callback when window event is received
		mRenderWindow->mWindowEvent.connect(std::bind(&RandomApp::handleWindowEvent, this, std::placeholders::_1));

		// Look for render targets, used to render into cloud and video textures
		mSunRenderTarget     = mResourceManager->findObject("SunRenderTarget");
		mVideoRenderTarget   = mResourceManager->findObject("VideoRenderTarget");
		mStaticRenderTarget  = mResourceManager->findObject("StaticRenderTarget");
		mPartyRenderTarget   = mResourceManager->findObject("PartyRenderTarget");
		mCombineRenderTarget = mResourceManager->findObject("CombineRenderTarget");

		// Look for Control Groups
		mControlGroups       = mResourceManager->findObject("ControlGroups");

		// All of our entities
		mScene = mResourceManager->findObject<Scene>("Scene");
		
		mSceneCamera = mScene->findEntity("SceneCamera");
		mSunClouds = mScene->findEntity("SunClouds");
		mSunGlare = mScene->findEntity("SunGlare");
		mOrthoCamera = mScene->findEntity("ProjectionCamera");
		mLightRig = mScene->findEntity("LightRig");
		mVideo = mScene->findEntity("Video");
		mStatic = mScene->findEntity("Static");
		mCombination = mScene->findEntity("Combination");
		mController = mScene->findEntity("Controller");
		mOrbit = mScene->findEntity("Orbit");
		mOrbitPath = mScene->findEntity("OrbitPath");
		mOrbitStart = mScene->findEntity("OrbitStart");
		mOrbitEnd = mScene->findEntity("OrbitEnd");
		mOrbitSun = mScene->findEntity("OrbitSun");
		mParty = mScene->findEntity("Party");

		// Set render states
		nap::RenderState render_state;
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;
		mRenderService->setRenderState(render_state);

		// Create Random GUI
		mGui = std::make_unique<RandomGui>(*this);

		return true;
	}

	void RandomApp::update(double deltaTime)
	{
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mSceneCamera.get() };
		mInputService->processAllEvents(input_router, entities);

		// Update Random App components
		mGui->update(deltaTime);
	}


	void RandomApp::render()
	{
		// Clear all unnecessary GL resources
		mRenderService->destroyGLContextResources(std::vector<rtti::ObjectPtr<nap::RenderWindow>>({ mRenderWindow }));

		// Make render window active so we can use it's context and draw into it
		mRenderWindow->makeActive();
		
		// Get orthographic camera. This camera renders in pixel space, starting at 0,0
		OrthoCameraComponentInstance& ortho_cam = mOrthoCamera->getComponent<OrthoCameraComponentInstance>();

		// Get Lighting Mode Component
		LightingModeComponentInstance& lightingModeComponent = mController->getComponent<LightingModeComponentInstance>();

		// Render lighting mode textures into back-buffer
		if (lightingModeComponent.isLightingModeRendered(LightingModes::Sun))
			renderSun(ortho_cam);
		if (lightingModeComponent.isLightingModeRendered(LightingModes::Video))
			renderVideo(ortho_cam);
		if (lightingModeComponent.isLightingModeRendered(LightingModes::Static))
			renderStatic(ortho_cam);
		if (lightingModeComponent.isLightingModeRendered(LightingModes::Party))
			renderParty(ortho_cam);

		// Render combination into back buffer
		renderCombination(ortho_cam);

		// We have now rendered the clouds and video into separate textures. 
		// These can be applied to the mesh visualization mesh. This is the mesh that is drawn to screen
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());
		{
			// Clear window 
			mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

			// Find the scene (perspective camera)
			nap::PerspCameraComponentInstance& camera = mSceneCamera->getComponent<nap::PerspCameraComponentInstance>();

			// Find components to render (Light rig, orbit)
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			mLightRig->getComponentsOfTypeRecursive<RenderableComponentInstance>(components_to_render);
			if (lightingModeComponent.isLightingModeSelected(LightingModes::Sun))
			{
				components_to_render.emplace_back(&mOrbitPath->getComponent<nap::RenderableMeshComponentInstance>());
				components_to_render.emplace_back(&mOrbitStart->getComponent<nap::RenderableMeshComponentInstance>());
				components_to_render.emplace_back(&mOrbitEnd->getComponent<nap::RenderableMeshComponentInstance>());
				components_to_render.emplace_back(&mOrbitSun->getComponent<nap::RenderableMeshComponentInstance>());
			}

			// Render components in one pass
			mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);
		}
			
		// Draw gui
		mGui->draw();

		// Swap to front
		mRenderWindow->swap();
	}


	void RandomApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		if (windowEvent.get_type().is_derived_from(RTTI_OF(WindowResizedEvent)))
		{
			const WindowResizedEvent& res_event = static_cast<const WindowResizedEvent&>(windowEvent);
			windowSize.x = res_event.mX;
			windowSize.y = res_event.mY;
		}
	}


	void RandomApp::renderSun(OrthoCameraComponentInstance& orthoCamera)
	{
		mRenderService->clearRenderTarget(mSunRenderTarget->getTarget());

		// Find the projection plane and render it to the back-buffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&mSunClouds->getComponent<nap::RenderableMeshComponentInstance>());
		components_to_render.emplace_back(&mSunGlare->getComponent<nap::RenderableMeshComponentInstance>());

		// Render sun plane to sun texture
		mRenderService->renderObjects(mSunRenderTarget->getTarget(), orthoCamera, components_to_render);
	}


	void RandomApp::renderVideo(OrthoCameraComponentInstance& orthoCamera)
	{
		mRenderService->clearRenderTarget(mVideoRenderTarget->getTarget());

		// Find the video plane and render it to the back-buffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&(mVideo->getComponent<nap::RenderableMeshComponentInstance>()));

		// Render video plane to video texture
		mRenderService->renderObjects(mVideoRenderTarget->getTarget(), orthoCamera, components_to_render);
	}


	void RandomApp::renderStatic(OrthoCameraComponentInstance& orthoCamera)
	{
		mRenderService->clearRenderTarget(mStaticRenderTarget->getTarget());

		// Find the static plane and render it to the back-buffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&(mStatic->getComponent<nap::RenderableMeshComponentInstance>()));

		// Render static plane to static texture
		mRenderService->renderObjects(mStaticRenderTarget->getTarget(), orthoCamera, components_to_render);
	}


	void RandomApp::renderParty(OrthoCameraComponentInstance& orthoCamera)
	{
		mRenderService->clearRenderTarget(mPartyRenderTarget->getTarget());

		// Find the static plane and render it to the back-buffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&(mParty->getComponent<nap::RenderableMeshComponentInstance>()));

		// Render static plane to static texture
		mRenderService->renderObjects(mPartyRenderTarget->getTarget(), orthoCamera, components_to_render);
	}


	void RandomApp::renderCombination(OrthoCameraComponentInstance& orthoCamera)
	{
		// Render combination texture into back-buffer (ie: video / clouds into separate texture)
		// Note that this also starts the download of the gpu texture into the bitmap in the background
		RenderCombinationComponentInstance& render_comp = mCombination->getComponent<RenderCombinationComponentInstance>();
		render_comp.render(orthoCamera);
	}


	int RandomApp::shutdown()
	{
		mGui.reset(nullptr);
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
