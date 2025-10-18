/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewapplet.h"
#include "meshpreviewloadcomponent.h"

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
#include <meshutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::MeshPreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace napkin 
{

	MeshPreviewApplet::MeshPreviewApplet(nap::Core& core) :
		napkin::Applet(core) { }

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
		mRenderAdvancedService = getCore().getService<nap::RenderAdvancedService>();

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

		// Get the text entity
		mTextEntity = scene->findEntity("Text");
		if (!error.check(mTextEntity != nullptr, "Missing 'Text' entity"))
			return false;

		// Get the render entity
		mLoaderEntity = scene->findEntity("Loader");
		if (!error.check(mLoaderEntity != nullptr, "Mussing 'Loader' entity"))
			return false;

		// Fetch the two different cameras
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		if (!error.check(mPerspectiveCamEntity != nullptr, "Missing 'PerspectiveCamera' entity"))
			return false;

		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");
		if (!error.check(mOrthographicCamEntity != nullptr, "Missing 'OrthographicCamera' entity"))
			return false;

		mLightsEntity = scene->findEntity("Lights");
		if (!error.check(mLightsEntity != nullptr, "Missing 'Lights' entity"))
			return false;

		// Create GUI
		mGUI = std::make_unique<MeshPreviewAppletGUI>(*this);
		mGUI->init();

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

		// Update GUI
		mGUI->update(deltaTime);
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

			// Render selected mesh
			auto& controller = mLoaderEntity->getComponent<MeshPreviewLoadComponentInstance>();
			if (controller.hasMesh())
			{
				controller.draw();
			}
			else
			{
				// Locate component that can render text to screen
				auto& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();
				render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
				render_text.draw(render_window);
			}

			// Draw our GUI
			mGUI->draw();

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
}
