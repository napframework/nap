/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "texturepreviewapplet.h"
#include "loadtexturecomponent.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <apicomponent.h>
#include <perspcameracomponent.h>
#include "texturepreviewappletgui.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::TexturePreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace napkin
{
	TexturePreviewApplet::TexturePreviewApplet(Core& core) :
		napkin::Applet(core), mGui(std::make_unique<TexturePreviewAppletGUI>(*this))
	{ }


	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool TexturePreviewApplet::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService = getCore().getService<nap::SceneService>();
		mInputService = getCore().getService<nap::InputService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Fetch render window
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "Missing 'Window'"))
			return false;

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(scene != nullptr, "Missing 'Scene'"))
			return false;

		// Fetch entities
		mTextEntity = scene->findEntity("TextEntity");
		if (!error.check(mTextEntity != nullptr, "Missing 'TextEntity'"))
			return false;

		mAPIEntity = scene->findEntity("APIEntity");
		if (!error.check(mAPIEntity != nullptr, "Missing 'APIEntity'"))
			return false;

		m2DTextureEntity = scene->findEntity("2DTextureEntity");
		if (!error.check(m2DTextureEntity != nullptr, "Missing '2DTextureEntity'"))
			return false;

		m2DOrthoCameraEntity = scene->findEntity("2DOrthoCameraEntity");
		if (!error.check(m2DOrthoCameraEntity != nullptr, "Missing 'OrthoCameraEntity'"))
			return false;

		mCubePerspCameraEntity = scene->findEntity("CubePerspCameraEntity");
		if (!error.check(mCubePerspCameraEntity != nullptr, "Missing 'CubePerspCameraEntity'"))
			return false;

		mCubeTextureEntity = scene->findEntity("CubeTextureEntity");
		if (!error.check(mCubePerspCameraEntity != nullptr, "Missing 'CubeTextureEntity'"))
			return false;

		// Set data directory to resolve texture load cmds against
		// TODO: This should be available to the component directly, exposed as an extension to core...
		auto* load_tex_comp = mAPIEntity->findComponent<LoadTextureComponentInstance>();
		if (!error.check(load_tex_comp != nullptr, "Missing 'LoadTextureComponent'"))
			return false;
		load_tex_comp->mProjectDataDirectory = getEditorInfo().getDataDirectory();

		return true;
	}

	
	// Update app
	void TexturePreviewApplet::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		static nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the in-use camera
		std::vector<nap::EntityInstance*> entities;
		auto& tex_controller = mAPIEntity->getComponent<LoadTextureComponentInstance>();
		switch (tex_controller.getType())
		{
			case LoadTextureComponentInstance::EType::Texture2D:
				mInputService->processWindowEvents(*mRenderWindow, input_router, { m2DOrthoCameraEntity.get() });
				break;
			case LoadTextureComponentInstance::EType::Cubemap:
				mInputService->processWindowEvents(*mRenderWindow, input_router, { mCubePerspCameraEntity.get() });
				break;
			case LoadTextureComponentInstance::EType::None:
				break;
			default:
				assert(false);
				break;
		}

		// update (create) gui
		mGui->update(deltaTime);
	}
	
	
	// Render app
	void TexturePreviewApplet::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Our scene contains a `nap::CubeMapFromFile` which must be pre-rendered in a headless render pass. This only needs to happen once, and there
		// are no other objects that require headless rendering each frame. Therefore, `isHeadlessCommandQueued` should only be true in the first frame
		// of rendering and record pre-render operations for our cube map resources.
		if (mRenderService->isHeadlessCommandQueued())
		{
			if (mRenderService->beginHeadlessRecording())
				mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		nap::RenderWindow& render_window = *mRenderWindow;
		render_window.setClearColor(mClearColor);
		if (mRenderService->beginRecording(render_window))
		{
			// Begin the render pass
			render_window.beginRendering();

			// Draw 2D texture or cubemap based on loaded type
			auto& tex_controller = mAPIEntity->getComponent<LoadTextureComponentInstance>();
			switch (tex_controller.getType())
			{
				case LoadTextureComponentInstance::EType::Texture2D:
				{
					// Draw texture using zoom-pan control
					auto& ortho_2d_cam = m2DOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>();
					auto& tex2d_com = m2DTextureEntity->getComponent<RenderableMeshComponentInstance>();
					mRenderService->renderObjects(*mRenderWindow, ortho_2d_cam, { &tex2d_com });
					break;
				}
				case LoadTextureComponentInstance::EType::Cubemap:
				{
					// Draw cubemap using orbit control
					auto& persp_cube_cam = mCubePerspCameraEntity->getComponent<PerspCameraComponentInstance>();
					auto& sky_com = mCubeTextureEntity->getComponent<RenderableComponentInstance>();
					mRenderService->renderObjects(*mRenderWindow, persp_cube_cam, { &sky_com });
					break;
				}
				case LoadTextureComponentInstance::EType::None:
				{
					// Otherwise notify user we can select a texture
					Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();
					render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
					render_text.draw(render_window);
					break;
				}
				default:
				{
					assert(false);
					break;
				}
			}

			// Render gui to screen
			mGui->draw();

			// End the render pass
			render_window.endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	

	void TexturePreviewApplet::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void TexturePreviewApplet::inputMessageReceived(InputEventPtr inputEvent)
	{
		mInputService->addEvent(std::move(inputEvent));
	}
}
