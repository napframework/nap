/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "texturepreviewappletgui.h"
#include "texturepreviewapicomponent.h"
#include "texturepreviewapplet.h"

// External includes
#include <vulkan/vk_enum_string_helper.h>
#include <imgui/imgui.h>
#include <imguiutils.h>

namespace napkin
{
	void TexturePreviewAppletGUI::update(double elapsedTime)
	{
		// Select applet window
		mApplet.mGuiService->selectWindow(mApplet.mRenderWindow);

		// Fetch loaded texture
		auto& tex_controller = mApplet.mAPIEntity->getComponent<TexturePreviewAPIComponentInstance>();
		auto* loaded_tex = tex_controller.getTexture();

		// Setup GUI for window
		ImGui::BeginMainMenuBar();

		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mApplet.mClearColor.getData());
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Details", loaded_tex != nullptr))
		{
			ImGui::PushID(loaded_tex);
			texDetail("Identifier", loaded_tex->mID);
			texDetail("Plane Width", utility::stringFormat("%d", loaded_tex->getDescriptor().getWidth()), "texel(s)");
			texDetail("Plane Height", utility::stringFormat("%d", loaded_tex->getDescriptor().getHeight()), "texel(s)");
			texDetail("Channels", RTTI_OF(nap::ESurfaceChannels), loaded_tex->getDescriptor().getChannels());
			texDetail("No. Channels", utility::stringFormat("%d", loaded_tex->getDescriptor().getNumChannels()));
			texDetail("Surface type", RTTI_OF(nap::ESurfaceDataType), loaded_tex->getDescriptor().getDataType());
			texDetail("Channel size", utility::stringFormat("%d", loaded_tex->getDescriptor().getChannelSize()), "byte(s)");
			texDetail("Pixel size", utility::stringFormat("%d", loaded_tex->getDescriptor().getBytesPerPixel()), "byte(s)");
			texDetail("Surface size", utility::stringFormat("%d", loaded_tex->getDescriptor().getSizeInBytes()), "byte(s)");
			texDetail("Pitch", utility::stringFormat("%d", loaded_tex->getDescriptor().getPitch()), "byte(s)");
			texDetail("Layers", utility::stringFormat("%d", loaded_tex->getLayerCount()));
			texDetail("Mip levels", utility::stringFormat("%d", loaded_tex->getMipLevels()));
			texDetail("Format", utility::stringFormat(string_VkFormat(loaded_tex->getFormat())));
			ImGui::PopID();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Texture", loaded_tex != nullptr))
		{
			switch (tex_controller.getType())
			{
				case TexturePreviewAPIComponentInstance::EType::Cubemap:
				{
					updateTextureCube(tex_controller);
					break;
				}
				case TexturePreviewAPIComponentInstance::EType::Texture2D:
				{
					updateTexture2D(tex_controller);
					break;
				}
				default:
					break;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", mApplet.getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", elapsedTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		// Add frame icon
		if (loaded_tex != nullptr &&
			ImGui::ImageButton(mApplet.mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame Texture"))
		{
			// Frame object & reset rotation
			tex_controller.frame();
		}
		ImGui::EndMainMenuBar();
	}


	void TexturePreviewAppletGUI::draw()
	{
		mApplet.mGuiService->draw();
	}


	void TexturePreviewAppletGUI::texDetail(std::string&& label, const std::string& value, std::string&& appendix)
	{
		float xoff = gui::dpi * mApplet.mGuiService->getScale() * 1.25f;
		float yoff = xoff * 2.0f;
		ImGui::TextColored(mApplet.mGuiService->getPalette().mFront3Color, label.c_str());
		ImGui::SameLine(xoff);
		ImGui::Text(value.c_str());
		if (!appendix.empty())
		{
			ImGui::SameLine(yoff);
			ImGui::TextColored(mApplet.mGuiService->getPalette().mFront1Color, appendix.c_str());
		}
	}


	void TexturePreviewAppletGUI::texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument)
	{
		assert(enumerator.is_enumeration());
		texDetail(std::move(label), enumerator.get_enumeration().value_to_name(argument).data());
	}


	void TexturePreviewAppletGUI::updateTexture2D(TexturePreviewAPIComponentInstance& controller)
	{
		// Show and set display mode
		static const std::array<const char*, 2> mode_labels = { "Zoom & Pan", "3D Mesh" };
		int display_idx = static_cast<int>(controller.mLoad2DComponent->getMode());
		if (ImGui::Combo("Display Mode", &display_idx, mode_labels.data(), mode_labels.size()))
		{
			auto display_mode = static_cast<TexturePreviewLoad2DComponentInstance::EMode>(display_idx);
			controller.mLoad2DComponent->setMode(display_mode);
			controller.frame();
		}

		// Update based on selected 2D texture display mode
		switch (static_cast<TexturePreviewLoad2DComponentInstance::EMode>(display_idx))
		{
			case TexturePreviewLoad2DComponentInstance::EMode::Plane:
			{
				// Texture opacity
				float opacity = controller.getOpacity();
				if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f))
					controller.setOpacity(opacity);
				break;
			}
			case TexturePreviewLoad2DComponentInstance::EMode::Mesh:
			{
				// Mesh selection labels
				const auto& meshes = controller.mLoad2DComponent->getMeshes();
				std::vector<const char*> labels; labels.reserve(meshes.size());
				for (const auto& mesh : meshes)
					labels.emplace_back(mesh.getMesh().mID.c_str());

				// Add mesh combo selection
				int current_idx = controller.mLoad2DComponent->getMeshIndex();
				if (ImGui::Combo("Preview Mesh", &current_idx, labels.data(), meshes.size()))
				{
					controller.mLoad2DComponent->setMeshIndex(current_idx);
					controller.frame();
				}

				// Mesh rotation
				float rotate_speed = controller.getRotate();
				if (ImGui::SliderFloat("Rotation Speed", &rotate_speed, 0.0f, 1.0f, "%.3f", 2.0f))
					controller.setRotate(rotate_speed);
				break;
			}
		default:
			break;
		}
	}


	void TexturePreviewAppletGUI::updateTextureCube(TexturePreviewAPIComponentInstance& controller)
	{
		// Mesh visibility
		auto& render_mesh_comp = *controller.mLoadCubeComponent->mMeshRenderer;
		bool visible = render_mesh_comp.isVisible();
		if (ImGui::Checkbox("Show Mesh", &visible))
			render_mesh_comp.setVisible(!render_mesh_comp.isVisible());

		const auto& meshes = controller.mLoadCubeComponent->getMeshes();
		std::vector<const char*> labels; labels.reserve(meshes.size());
		for (const auto& mesh : meshes)
			labels.emplace_back(mesh.getMesh().mID.c_str());

		// Add mesh combo selection
		int current_idx = controller.mLoadCubeComponent->getMeshIndex();
		if (ImGui::Combo("Preview Mesh", &current_idx, labels.data(), meshes.size()))
		{
			controller.mLoadCubeComponent->setMeshIndex(current_idx);
			controller.frame();
		}

		// Skybox opacity
		float opacity = controller.getOpacity();
		if (ImGui::SliderFloat("Skybox Opacity", &opacity, 0.0f, 1.0f))
			controller.setOpacity(opacity);

		// Mesh rotation
		float rotate_speed = controller.getRotate();
		if (ImGui::SliderFloat("Rotation Speed", &rotate_speed, 0.0f, 1.0f, "%.3f", 2.0f))
			controller.setRotate(rotate_speed);
	}
}
