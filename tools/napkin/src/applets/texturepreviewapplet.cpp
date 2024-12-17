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
#include <vulkan/vk_enum_string_helper.h>
#include <imguiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::TexturePreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace napkin
{
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

		mTextEntity = scene->findEntity("TextEntity");
		if (!error.check(mTextEntity != nullptr, "Missing 'TextEntity'"))
			return false;

		mAPIEntity = scene->findEntity("APIEntity");
		if (!error.check(mAPIEntity != nullptr, "Missing 'APIEntity'"))
			return false;

		m2DTextureEntity = scene->findEntity("2DTextureEntity");
		if (!error.check(m2DTextureEntity != nullptr, "Missing '2DTextureEntity'"))
			return false;

		mOrthoEntity = scene->findEntity("OrthoCameraEntity");
		if (!error.check(mOrthoEntity != nullptr, "Missing 'OrthoCameraEntity'"))
			return false;

		auto* load_tex_comp = mAPIEntity->findComponent<LoadTextureComponentInstance>();
		if (!error.check(load_tex_comp != nullptr, "Missing 'LoadTextureComponent'"))
			return false;

		// Set data directory to resolve texture load cmds against
		// TODO: This should be available to the component directly, exposed as an extension to core...
		load_tex_comp->mProjectDataDirectory = getEditorInfo().getDataDirectory();

		return true;
	}

	
	// Update app
	void TexturePreviewApplet::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;

		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mOrthoEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Get texture
		auto* loaded_texture = mAPIEntity->getComponent<LoadTextureComponentInstance>().getTexture();

		// Setup GUI
		ImGui::BeginMainMenuBar();
		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mClearColor.getData());	
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Details", loaded_texture != nullptr))
		{
			ImGui::PushID(loaded_texture);
			texDetail("Width", utility::stringFormat("%d", loaded_texture->getWidth()), "texel(s)");
			texDetail("Height", utility::stringFormat("%d", loaded_texture->getHeight()), "texel(s)");
			texDetail("Channels", RTTI_OF(nap::ESurfaceChannels), loaded_texture->getDescriptor().getChannels());
			texDetail("No. Channels", utility::stringFormat("%d", loaded_texture->getDescriptor().getNumChannels()));
			texDetail("Surface type", RTTI_OF(nap::ESurfaceDataType), loaded_texture->getDescriptor().getDataType());
			texDetail("Channel size", utility::stringFormat("%d", loaded_texture->getDescriptor().getChannelSize()), "byte(s)");
			texDetail("Pixel size", utility::stringFormat("%d", loaded_texture->getDescriptor().getBytesPerPixel()), "byte(s)");
			texDetail("Surface size", utility::stringFormat("%d", loaded_texture->getDescriptor().getSizeInBytes()), "byte(s)");
			texDetail("Pitch", utility::stringFormat("%d", loaded_texture->getDescriptor().getPitch()), "byte(s)");
			texDetail("Layers", utility::stringFormat("%d", loaded_texture->getLayerCount()));
			texDetail("Mip levels", utility::stringFormat("%d", loaded_texture->getMipLevels()));
			texDetail("Format", utility::stringFormat(string_VkFormat(loaded_texture->getFormat())));
			ImGui::PopID();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Controls", loaded_texture != nullptr))
		{
			ImGui::PushID(loaded_texture);
			static float scale;
			ImGui::SliderFloat("UV Scale", &scale, 0.0f, 10.0f, "%.3f", 2.0f);
			ImGui::PopID();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		// Add frame icon
		if (loaded_texture != nullptr &&
			ImGui::ImageButton(mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame"))
		{
			auto& frame_2d_comp = m2DTextureEntity->getComponent<napkin::Frame2DTextureComponentInstance>();
			frame_2d_comp.frame();
		}
		ImGui::EndMainMenuBar();
	}
	
	
	// Render app
	void TexturePreviewApplet::render()
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

			// Get 2D texture and draw
			if (mAPIEntity->getComponent<LoadTextureComponentInstance>().hasTexture())
			{
				auto& ortho_cam = mOrthoEntity->getComponent<OrthoCameraComponentInstance>();
				auto& tex2d_com = m2DTextureEntity->getComponent<RenderableMeshComponentInstance>();
				mRenderService->renderObjects(*mRenderWindow, ortho_cam, { &tex2d_com });
			}
			else
			{
				// Otherwise notify user we can select a texture
				Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();
				render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
				render_text.draw(render_window);
			}

			// Render gui to screen
			mGuiService->draw();

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


	void TexturePreviewApplet::texDetail(std::string&& label, std::string&& value, std::string&& appendix)
	{
		static constexpr float xoff = 125.0f;
		static constexpr float yoff = xoff * 2.0f;
		ImGui::TextColored(mGuiService->getPalette().mFront3Color, label.c_str());
		ImGui::SameLine(xoff);
		ImGui::Text(value.c_str());
		if (!appendix.empty())
		{
			ImGui::SameLine(yoff);
			ImGui::TextColored(mGuiService->getPalette().mFront1Color, appendix.c_str());
		}
	}


	void TexturePreviewApplet::texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument)
	{
		assert(enumerator.is_enumeration());
		texDetail(label.c_str(), enumerator.get_enumeration().value_to_name(argument).data());
	}
}
