#include "renderpreviewapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderPreviewApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool RenderPreviewApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService = getCore().getService<nap::SceneService>();
		mInputService = getCore().getService<nap::InputService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mWorldTexture = mResourceManager->findObject<nap::ImageFromFile>("PreviewWorldTexture");
		if (!error.check(mWorldTexture != nullptr, "Missing 'PreviewWorldTexture'"))
			return false;

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("PreviewScene");
		if (!error.check(scene != nullptr, "Missing 'PreviewScene'"))
			return false;

		// Fetch world and text
		mWorldEntity = scene->findEntity("PreviewWorld");
		if (!error.check(mWorldEntity != nullptr, "Missing 'PreviewWorldEntity'"))
			return false;

		mTextEntity = scene->findEntity("PreviewText");
		if (!error.check(mTextEntity != nullptr, "Missing 'PreviewTextEntity'"))
			return false;

		// Fetch the two different cameras
		mPerspectiveCamEntity = scene->findEntity("PreviewPerspectiveCamera");
		if (!error.check(mPerspectiveCamEntity != nullptr, "Missing 'PreviewPerspectiveCamera'"))
			return false;

		mOrthographicCamEntity = scene->findEntity("PreviewOrthographicCamera");
		if (!error.check(mOrthographicCamEntity != nullptr, "Missing 'PreviewOrthographicCamera'"))
			return false;

		// Sample default color values from loaded color palette
		mColorTwo = RGBColor8(0x29, 0x58, 0xff).convert<RGBColorFloat>();
		mColorOne = { mColorTwo[0] * 0.9f, mColorTwo[1] * 0.9f, mColorTwo[2] };
		mHaloColor = RGBColor8(0xFF, 0xFF, 0xFF).convert<RGBColorFloat>();
		mTextColor = mHaloColor;

		return true;
	}
	
	
	// Update app
	void RenderPreviewApp::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;

		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(getRenderWindow(), input_router, entities);

		// Push the current color selection to the shader.
		nap::RenderableMeshComponentInstance& renderer = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		auto ubo = renderer.getMaterialInstance().getOrCreateUniform("UBO");
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("colorOne")->setValue(mColorOne);
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("colorTwo")->setValue(mColorTwo);
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("haloColor")->setValue(mHaloColor);

		// Push text color
		auto& text_comp = mTextEntity->getComponent<Renderable2DTextComponentInstance>();
		text_comp.setColor(mTextColor);
	}
	
	
	// Render app
	void RenderPreviewApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		nap::RenderWindow& render_window = getRenderWindow();

		if (mRenderService->beginRecording(render_window))
		{
			// Begin the render pass
			render_window.beginRendering();

			// Find the world and add as an object to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&renderable_world);

			// Find the perspective camera
			nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(render_window, persp_camera, components_to_render);

			// Locate component that can render text to screen
			Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();

			// Center text and render it using the given draw call, 
			// alternatively you can use an orthographic camera to render the text, similar to how the 3D mesh is rendered:  
			// mRenderService::renderObjects(*mRenderWindow, ortho_camera, components_to_render);
			render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
			render_text.draw(render_window);

			// End the render pass
			render_window.endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	

	void RenderPreviewApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void RenderPreviewApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int RenderPreviewApp::shutdown()
	{
		return 0;
	}

}
