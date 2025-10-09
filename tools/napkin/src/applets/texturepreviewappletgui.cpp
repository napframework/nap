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

		if (ImGui::BeginMenu("Texture", loaded_tex != nullptr))
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

		if (ImGui::BeginMenu("Mesh", loaded_tex != nullptr))
		{
			// Mesh attributes
			auto* mesh = tex_controller.getMesh(); assert(mesh != nullptr);
			const auto& mesh_instance = mesh->getMeshInstance();
			ImGui::PushID(mesh);
			texDetail("Identifier", mesh->mID);
			texDetail("Vertices", utility::stringFormat("%d", mesh_instance.getNumVertices()));
			texDetail("Shapes", utility::stringFormat("%d", mesh_instance.getNumShapes()));
			texDetail("Cull Mode", RTTI_OF(ECullMode), mesh_instance.getCullMode());
			texDetail("Topology", RTTI_OF(EDrawMode), mesh_instance.getDrawMode());
			texDetail("Polygon Mode ", RTTI_OF(EPolygonMode), mesh_instance.getPolygonMode());
			texDetail("Usage", RTTI_OF(EMemoryUsage), mesh_instance.getUsage());

			// Vertex attributes
			texDetail("Attributes", utility::stringFormat("%d", mesh_instance.getAttributes().size()));
			for (auto i = 0; i < mesh_instance.getAttributes().size(); i++)
			{
				auto& attr = *mesh_instance.getAttributes()[i];
				ImGui::PushID(&attr);
				texDetail(utility::stringFormat("\t%d", i), attr.mAttributeID,
					nap::utility::stripNamespace(attr.getElementType().get_name().data()));
				ImGui::PopID();
			}

			// Mesh bound dimensions
			const auto& bounds = tex_controller.getMeshBounds();
			texDetail("Bounds", "");
			texDetail("\tWidth", utility::stringFormat("%.1f", bounds.getWidth()), "unit(s)");
			texDetail("\tHeight", utility::stringFormat("%.1f", bounds.getHeight()), "unit(s)");
			texDetail("\tDepth", utility::stringFormat("%.1f", bounds.getDepth()), "unit(s)");

			// Mesh bound coordinates
			static constexpr const float cutoff = 0.01;
			static constexpr const glm::vec3 zero = { 0.0f, 0.0f, 0.0f };
			auto min = glm::length(bounds.getMin()) < cutoff ? zero : bounds.getMin();
			auto max = glm::length(bounds.getMax()) < cutoff ? zero : bounds.getMax();
			auto cen = glm::length(bounds.getCenter()) < cutoff ? zero : bounds.getCenter();
			texDetail("\tMin", utility::stringFormat("%.1f, %.1f, %.1f", min.x, min.y, min.z));
			texDetail("\tMax", utility::stringFormat("%.1f, %.1f, %.1f", max.x, max.y, max.z));
			texDetail("\tCenter", utility::stringFormat("%.1f, %.1f, %.1f", cen.x, cen.y, cen.z));

			ImGui::PopID();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Controls", loaded_tex != nullptr))
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
			ImGui::ImageButton(mApplet.mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame Selection"))
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
		int display_idx = static_cast<int>(controller.mFrame2DTextureComponent->getMode());
		if (ImGui::Combo("Display Mode", &display_idx, mode_labels.data(), mode_labels.size()))
		{
			auto display_mode = static_cast<Frame2DTextureComponentInstance::EMode>(display_idx);
			controller.mFrame2DTextureComponent->setMode(display_mode);
			controller.frame();
		}

		// Update based on selected 2D texture display mode
		switch (static_cast<Frame2DTextureComponentInstance::EMode>(display_idx))
		{
			case Frame2DTextureComponentInstance::EMode::Plane:
			{
				// Texture opacity
				float opacity = controller.getOpacity();
				if (ImGui::SliderFloat("Texture Opacity", &opacity, 0.0f, 1.0f))
					controller.setOpacity(opacity);
				break;
			}
			case Frame2DTextureComponentInstance::EMode::Mesh:
			{
				// Mesh selection labels
				const auto& meshes = controller.mFrame2DTextureComponent->getMeshes();
				std::vector<const char*> labels; labels.reserve(meshes.size());
				for (const auto& mesh : meshes)
					labels.emplace_back(mesh.getMesh().mID.c_str());

				// Add mesh combo selection
				int current_idx = controller.mFrame2DTextureComponent->getMeshIndex();
				if (ImGui::Combo("Preview Mesh", &current_idx, labels.data(), meshes.size()))
				{
					controller.mFrame2DTextureComponent->setMeshIndex(current_idx);
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
		auto& render_mesh_comp = *controller.mFrameCubeComponent->mMeshRenderer;
		bool visible = render_mesh_comp.isVisible();
		if (ImGui::Checkbox("Show Mesh", &visible))
			render_mesh_comp.setVisible(!render_mesh_comp.isVisible());

		const auto& meshes = controller.mFrameCubeComponent->getMeshes();
		std::vector<const char*> labels; labels.reserve(meshes.size());
		for (const auto& mesh : meshes)
			labels.emplace_back(mesh.getMesh().mID.c_str());

		// Add mesh combo selection
		int current_idx = controller.mFrameCubeComponent->getMeshIndex();
		if (ImGui::Combo("Preview Mesh", &current_idx, labels.data(), meshes.size()))
		{
			controller.mFrameCubeComponent->setMeshIndex(current_idx);
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
