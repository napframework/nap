#include "multiwindowapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imguiutils.h>
#include <uniforminstances.h>
#include <sdlhelpers.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MultiWindowApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool MultiWindowApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService  = getCore().getService<nap::SceneService>();
		mInputService  = getCore().getService<nap::InputService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("multiwindow.json", error))
			return false;

		// Get screen size
		glm::ivec2 screen_size = SDL::getScreenSize(0);
		
		// Calculate x and y window offsets
		int offset_x = (screen_size.x - (3 * 512)) / 2;
		int offset_y = (screen_size.y - 512) / 2;

		// Extract windows and set position
		mRenderWindowOne = mResourceManager->findObject<nap::RenderWindow>("Window0");
		mRenderWindowOne->setPosition({ offset_x, offset_y });

		mRenderWindowTwo = mResourceManager->findObject<nap::RenderWindow>("Window1");
		mRenderWindowTwo->setPosition({ offset_x + 512, offset_y });
		
		mRenderWindowThree = mResourceManager->findObject<nap::RenderWindow>("Window2");
		mRenderWindowThree->setPosition({ offset_x + 1024, offset_y });

		// Extract textures
		mTextureOne = mResourceManager->findObject<ImageFromFile>("TextureOne");
		mTextureTwo = mResourceManager->findObject<ImageFromFile>("TextureTwo");
		mWorldTexture = mResourceManager->findObject<ImageFromFile>("WorldTexture");

		// Find the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mWorldEntity = scene->findEntity("World");
		mPerspectiveCameraOne = scene->findEntity("PerpectiveCameraOne");
		mPerspectiveCameraTwo = scene->findEntity("PerpectiveCameraTwo");
		mOrthoCamera = scene->findEntity("OrthoCamera");
		mPlaneOneEntity = scene->findEntity("PlaneOne");
		mPlaneTwoEntity = scene->findEntity("PlaneTwo");

		OrthoCameraComponentInstance& ortho_comp = mOrthoCamera->getComponent<OrthoCameraComponentInstance>();
		return true;
	}
	

	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our two perspective cameras.
	 * See helloworld demo for more info on input handling
	 *
	 * positionPlane centers the plane for both windows.
	 */
	void MultiWindowApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCameraOne.get() };
		mInputService->processWindowEvents(*mRenderWindowOne, input_router, entities);

		// Forward all input events associated with the third window to listening components
		entities.clear();
		entities.emplace_back(mPerspectiveCameraTwo.get());
		mInputService->processWindowEvents(*mRenderWindowThree, input_router, entities);

		// Center the first plane relative to the orthographic camera
		// The orthographic camera works in pixel space, therefore we need to move the position
		// of the plane to the right and scale it based on the size of the window
		TransformComponentInstance& plane_xform_one = mPlaneOneEntity->getComponent<TransformComponentInstance>();
		positionPlane(*mRenderWindowTwo, plane_xform_one);
		
		// Do the same for the second plane that is drawn in the third window
		TransformComponentInstance& plane_xform_two = mPlaneTwoEntity->getComponent<TransformComponentInstance>();
		positionPlane(*mRenderWindowThree, plane_xform_two);

		// Update the gui for all windows
		updateGUI();
	}

	
	/**
	 * We render 2 objects to 3 different windows using various cameras
	 * First we render the sphere to the first window, this is exactly the same as in the helloworld demo
	 * After that we render the plane using texture 1 to the second window using the orthographic camera
	 * Finally we render the sphere together with an overlay texture in separate passes to the third window. 
	 * The sphere uses a different camera than the one used for the first window. We do this because we want to control it individually.
	 * The texture is also different, this one is transparent. 
	 */
	void MultiWindowApp::render()
	{
		// Find the camera uniform we need to set for both render passes that contain a sphere
		nap::RenderableMeshComponentInstance& render_mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformStructInstance* frag_ubo = render_mesh.getMaterialInstance().getOrCreateUniform("UBO");
		nap::UniformVec3Instance* cam_loc_uniform = frag_ubo->getOrCreateUniform<nap::UniformVec3Instance>("inCameraPosition");

		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Render Window One : Sphere
		if(mRenderService->beginRecording(*mRenderWindowOne))
		{
			// Begin the render pass
			mRenderWindowOne->beginRendering();

			// Find the world and add as an object to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&renderable_world);

			// Find the camera
			nap::PerspCameraComponentInstance& camera = mPerspectiveCameraOne->getComponent<nap::PerspCameraComponentInstance>();

			// Set the camera location uniform
			nap::TransformComponentInstance& cam_xform = mPerspectiveCameraOne->getComponent<nap::TransformComponentInstance>();
			glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
			cam_loc_uniform->setValue(global_pos);

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(mRenderWindowOne->getBackbuffer(), camera, components_to_render);

			// Draw gui to window one
			mGuiService->draw(*mRenderWindowOne);

			// End render pass
			mRenderWindowOne->endRendering();

			// End record pass
			mRenderService->endRecording();
		}

		// Render Window Two : Texture
		if(mRenderService->beginRecording(*mRenderWindowTwo))
		{
			// Begin render pass
			mRenderWindowTwo->beginRendering();

			// Find the plane entity and add as an object to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			nap::RenderableMeshComponentInstance& renderable_plane = mPlaneOneEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&renderable_plane);

			// Find the camera
			nap::OrthoCameraComponentInstance& camera = mOrthoCamera->getComponent<nap::OrthoCameraComponentInstance>();

			// Render the plane with the orthographic to window two
			mRenderService->renderObjects(mRenderWindowTwo->getBackbuffer(), camera, components_to_render);

			// Draw gui to window one
			mGuiService->draw(*mRenderWindowTwo);

			// End render pass
			mRenderWindowTwo->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Render Window Three: Sphere and Texture
		if(mRenderService->beginRecording(*mRenderWindowThree))
		{
			// Begin the render pass
			mRenderWindowThree->beginRendering();
			
			// Find the world entity and add as an object to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&renderable_world);

			// Find the second perspective camera
			nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCameraTwo->getComponent<nap::PerspCameraComponentInstance>();
			
			// Set the camera location uniform for the halo effect
			nap::TransformComponentInstance& cam_xform = mPerspectiveCameraTwo->getComponent<nap::TransformComponentInstance>();
			glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
			cam_loc_uniform->setValue(global_pos);

			// Render sphere
			mRenderService->renderObjects(mRenderWindowThree->getBackbuffer(), persp_camera, components_to_render);

			// Now find the second plane to render
			nap::RenderableMeshComponentInstance& renderable_plane = mPlaneTwoEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.clear();
			components_to_render.emplace_back(&renderable_plane);

			// Find the orthographic camera
			nap::OrthoCameraComponentInstance& camera = mOrthoCamera->getComponent<nap::OrthoCameraComponentInstance>();

			// Render the plane with the orthographic to window three
			mRenderService->renderObjects(mRenderWindowThree->getBackbuffer(), camera, components_to_render);

			// Draw gui to window one
			mGuiService->draw(*mRenderWindowThree);

			// Stop render pass
			mRenderWindowThree->endRendering();

			// Stop recording
			mRenderService->endRecording();
		}

		// Submit recorded commands
		mRenderService->endFrame();
	}


	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void MultiWindowApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void MultiWindowApp::inputMessageReceived(InputEventPtr inputEvent)
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
				nap::RenderWindow* window = mRenderService->findWindow(press_event->mWindow);
				window->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int MultiWindowApp::shutdown()
	{
		return 0;
	}


	void MultiWindowApp::positionPlane(nap::RenderWindow& window, nap::TransformComponentInstance& planeTransform)
	{
        glm::ivec2 pixel_size = window.getBackbuffer().getSize();
		float window_width = pixel_size.x;
		float window_heigh = pixel_size.y;

		// Scale of plane is smallest variant of window size
		float scale = 0.0f;
		glm::ivec2 offset(0.0f, 0.0f);
		if (window_width > window_heigh)
		{
			scale = window_heigh;
			offset.x = (window_width - window_heigh) / 2.0f;
		}
		else
		{
			scale = window_width;
			offset.y = (window_heigh - scale) / 2.0f;
		}

		float pos_x = (scale / 2.0f) + offset.x;
		float pos_y = (scale / 2.0f) + offset.y;
			 
		// Push position values to transform
		planeTransform.setTranslate(glm::vec3(pos_x, pos_y, 0.0f));
		planeTransform.setScale(glm::vec3(scale, scale, 1.0f));
	}


	void MultiWindowApp::updateGUI()
	{
		// Select window 1
		mGuiService->selectWindow(mRenderWindowOne);

		// Draw some GUI elements and show used textures
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Used Textures 1"))
		{
			float col_width = ImGui::GetColumnWidth();
			float ratio = (float)mWorldTexture->getHeight() / (float)mWorldTexture->getWidth();
			ImGui::Image(*mWorldTexture, ImVec2(col_width, col_width * ratio));
		}
		ImGui::End();
		ImGui::ShowDemoWindow(nullptr);

		// Select window 2
		mGuiService->selectWindow(mRenderWindowTwo);

		// Draw some GUI elements and show used textures
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(clr, "Howdy! How are you doing?");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Used Textures 2"))
		{
			float col_width = ImGui::GetColumnWidth();
			float ratio = (float)mTextureOne->getHeight() / (float)mTextureOne->getWidth();
			ImGui::Image(*mTextureOne, ImVec2(col_width, col_width * ratio));
		}
		ImGui::End();

		// Select window 3
		mGuiService->selectWindow(mRenderWindowThree);

		// Draw some GUI elements and show used textures
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Used Textures 3"))
		{
			// World texture
			float col_width = ImGui::GetColumnWidth();
			float ratio = (float)mWorldTexture->getHeight() / (float)mWorldTexture->getWidth();
			ImGui::Image(*mWorldTexture, ImVec2(col_width, col_width * ratio));

			// Texture two
			ratio = (float)mTextureTwo->getHeight() / (float)mTextureTwo->getWidth();
			ImGui::Image(*mTextureTwo, ImVec2(col_width, col_width * ratio));
		}
		ImGui::End();
	}
}
