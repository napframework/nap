/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewapplet.h"

// External includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <apicomponent.h>
#include <imguiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::MeshPreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace napkin 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool MeshPreviewApplet::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService = getCore().getService<nap::SceneService>();
		mInputService = getCore().getService<nap::InputService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Fetch render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "Missing 'Window'"))
			return false;

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(scene != nullptr, "Missing 'Scene'"))
			return false;

		mTextEntity = scene->findEntity("Text");
		if (!error.check(mTextEntity != nullptr, "Missing 'Text' Entity"))
			return false;

		// Fetch the two different cameras
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		if (!error.check(mPerspectiveCamEntity != nullptr, "Missing 'PerspectiveCamera' entity"))
			return false;

		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");
		if (!error.check(mOrthographicCamEntity != nullptr, "Missing 'OrthographicCamera' entity"))
			return false;

		return true;
	}
	
	
	// Update app
	void MeshPreviewApplet::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;

		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Setup GUI
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mClearColor.getData());
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;

		// Add frame icon
		if (ImGui::ImageButton(mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame Selection"))
		{
			// TODO: Frame
		}
		ImGui::EndMainMenuBar();
	}
	
	
	// Render app
	void MeshPreviewApplet::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		nap::RenderWindow& render_window = *mRenderWindow;
		render_window.setClearColor(mClearColor);

		if (mRenderService->beginRecording(render_window))
		{
			// Begin the render pass
			render_window.beginRendering();

			// Locate component that can render text to screen
			Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();

			// Center text and render it using the given draw call, 
			render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
			render_text.draw(render_window);

			// Draw our GUI
			mGuiService->draw();

			// End the render pass
			render_window.endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	

	void MeshPreviewApplet::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void MeshPreviewApplet::inputMessageReceived(InputEventPtr inputEvent)
	{
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int MeshPreviewApplet::shutdown()
	{
		return 0;
	}
}
