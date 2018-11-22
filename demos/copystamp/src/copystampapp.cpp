#include "copystampapp.h"
#include "renderablecopymeshcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include<imgui/imgui.h>
#include <imguiutils.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CopystampApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	bool CopystampApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("copystamp.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");	

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = scene->findEntity("Camera");
		mWorldEntity  = scene->findEntity("World");

		return true;
	}
	

	void CopystampApp::update(double deltaTime)
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update gui and check for gui changes
		updateGui();
	}

	
	void CopystampApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Get perspective camera
		PerspCameraComponentInstance& persp_camera = mCameraEntity->getComponent<PerspCameraComponentInstance>();

		mRenderService->setPolygonMode(opengl::EPolygonMode::Fill);

		// Get mesh to render
		RenderableCopyMeshComponentInstance& copy_mesh = mWorldEntity->getComponent<RenderableCopyMeshComponentInstance>();
		
		// Set camera in shader used by mesh
		TransformComponentInstance& cam_xform = mCameraEntity->getComponent<TransformComponentInstance>();
		UniformVec3& cam_loc_uniform = copy_mesh.getMaterial().getOrCreateUniform<UniformVec3>("cameraLocation");
		cam_loc_uniform.setValue(math::extractPosition(cam_xform.getGlobalTransform()));
		
		// Render all copied meshes
		std::vector<RenderableComponentInstance*> renderable_comps = { &copy_mesh };
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), persp_camera, renderable_comps);

		// Draw gui
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
	void CopystampApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void CopystampApp::inputMessageReceived(InputEventPtr inputEvent)
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
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int CopystampApp::shutdown()
	{
		return 0;
	}


	void CopystampApp::updateGui()
	{
		// Get component that copies meshes onto target mesh
		RenderableCopyMeshComponentInstance& copy_comp = mWorldEntity->getComponent<RenderableCopyMeshComponentInstance>();

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if(ImGui::CollapsingHeader("Controls"))
		{
			ImGui::SliderInt("Random Seed", &(copy_comp.mSeed), 0, 100);
			ImGui::SliderFloat("Global Scale", &(copy_comp.mScale), 0.0f, 2.0f);
			ImGui::SliderFloat("Random Scale", &(copy_comp.mRandomScale), 0.0f, 1.0f);
			ImGui::Checkbox("Orient", &(copy_comp.mOrient));
		}
		ImGui::End();
	}
}
