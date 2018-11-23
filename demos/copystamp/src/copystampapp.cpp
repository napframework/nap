// Local Includes
#include "copystampapp.h"
#include "renderablecopymeshcomponent.h"

// Nap includes
#include <nap/core.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CopystampApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool CopystampApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load copystamp json file
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
	

	/**
	* Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 *
	 * The camera has two input components: KeyInputComponent and PointerInputComponent
	 * The key input component receives key events, the pointer input component receives pointer events
	 * The orbit controller listens to both of them
	 * When an input component receives a message it sends a signal to the orbit controller.
	 * The orbit controller validates if it's something useful and acts accordingly,
	 * in this case by rotating around or zooming in on the sphere.
	 */
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

	
	/**
	 * The render service only renders our custom copy / stamp component (nap::RenderableCopyMeshComponent).
	 * All placement, orientation and drawing logic is handled inside that component.
	 */
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

		// Get mesh to render
		RenderableCopyMeshComponentInstance& copy_mesh = mWorldEntity->getComponent<RenderableCopyMeshComponentInstance>();
		
		// Set camera location in the shader that draws all the meshes.
		// The camera location is used for the light computation.
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
			ImGui::Checkbox("Look At Camera", &(copy_comp.mOrient));
			ImGui::SliderInt("Random Seed", &(copy_comp.mSeed), 0, 100);
			ImGui::SliderFloat("Global Scale", &(copy_comp.mScale), 0.0f, 2.0f);
			ImGui::SliderFloat("Rotation Speed", &(copy_comp.mRotationSpeed), 0.0f, 10.0f);
			ImGui::SliderFloat("Random Scale", &(copy_comp.mRandomScale), 0.0f, 1.0f);
		}
		ImGui::End();
	}
}
