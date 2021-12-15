/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imguiutils.h"
#include "imguiservice.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>

namespace ImGui
{
	void Image(nap::Texture2D& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		nap::Core& core = texture.getRenderService().getCore();
		nap::IMGuiService* gui_service = core.getService<nap::IMGuiService>();
		ImGui::Image(gui_service->getTextureHandle(texture), size, uv0, uv1, tint_col, border_col);
	}


	bool IMGUI_API ImageButton(nap::Texture2D& texture, const ImVec2& size, const ImVec2& uv0 /*= ImVec2(0, 1)*/, const ImVec2& uv1 /*= ImVec2(1, 0)*/, int frame_padding /*= -1*/, const ImVec4& bg_col /*= ImVec4(0, 0, 0, 0)*/, const ImVec4& tint_col /*= ImVec4(1, 1, 1, 1)*/)
	{
		nap::Core& core = texture.getRenderService().getCore();
		nap::IMGuiService* gui_service = core.getService<nap::IMGuiService>();
		return ImGui::ImageButton(gui_service->getTextureHandle(texture), size, uv0, uv1, frame_padding, bg_col, tint_col);
	}


	bool ImageButton(nap::Texture2D& texture, int frame_padding , const ImVec4& bg_col , const ImVec4& tint_col)
	{
		float size = ImGui::GetFontSize();
		return ImageButton(texture, { size, size }, { 0,1 }, { 1,0 }, frame_padding, bg_col, tint_col);
	}


	ImTextureID GetTextureHandle(nap::Texture2D& texture)
	{
		nap::Core& core = texture.getRenderService().getCore();
		nap::IMGuiService* gui_service = core.getService<nap::IMGuiService>();
		return gui_service->getTextureHandle(texture);
	}


	bool Combo(const char* label, int* current_item, nap::rtti::TypeInfo enum_type)
	{
		assert(enum_type.is_enumeration());
		rttr::enumeration enum_instance = enum_type.get_enumeration();
		std::vector<rttr::string_view> items(enum_instance.get_names().begin(), enum_instance.get_names().end());
		return ImGui::Combo(label, current_item, [](void* data, int index, const char** out_text)
		{
			std::vector<rttr::string_view>* items = (std::vector<rttr::string_view>*)data;
			*out_text = (*items)[index].data();
			return true;
		}, &items, items.size());
	}
}
