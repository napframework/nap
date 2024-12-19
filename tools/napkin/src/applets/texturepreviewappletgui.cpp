/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "texturepreviewappletgui.h"
#include "loadtexturecomponent.h"

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

		// Setup GUI for window
		ImGui::BeginMainMenuBar();
		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mApplet.mClearColor.getData());
			ImGui::EndMenu();
		}

		auto& tex_controller = mApplet.mAPIEntity->getComponent<LoadTextureComponentInstance>();
		auto* loaded_tex = tex_controller.getTexture();
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
		if (ImGui::BeginMenu("Controls", loaded_tex != nullptr))
		{
			ImGui::PushID(loaded_tex);
			static float scale;
			ImGui::SliderFloat("UV Scale", &scale, 0.0f, 10.0f, "%.3f", 2.0f);
			ImGui::PopID();
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
			ImGui::ImageButton(mApplet.mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame"))
			tex_controller.frame();

		ImGui::EndMainMenuBar();
	}


	void TexturePreviewAppletGUI::draw()
	{
		mApplet.mGuiService->draw();
	}


	void TexturePreviewAppletGUI::texDetail(std::string&& label, const std::string& value, std::string&& appendix)
	{
		static constexpr float xoff = 125.0f;
		static constexpr float yoff = xoff * 2.0f;
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
}
