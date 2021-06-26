/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "imgui/imgui.h"

// External Includes
#include <texture2d.h>
#include <utility/dllexport.h>
#include <rtti/typeinfo.h>

/**
 * This file contains NAP overrides for popular IMGui functions
 * These utility functions allow you to use common NAP objects in conjunction with IMGui
 * All the functions in this file follow the IMGui style and naming conventions
 */
namespace ImGui
{
	/**
	 * Displays a NAP 2D texture as an IMGUI image
	 * @param texture the texture to display in IMGUI
	 * @param size display size of the texture in pixels
	 * @param uv0 the min uv coordinates, defaults to lower left corner
	 * @param uv1 the max uv coordinates, defaults to upper right corner
	 * @param tint_col used to tint the displayed texture
	 * @param border_col of the border of the image
	 */
	void IMGUI_API Image(nap::Texture2D& texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

	/**
	 * Return ImTextureID, can be used when drawing textures in a window drawlist
	 * @param texture the texture to retrieve the ImTextureID from
	 * @return the ImTextureID
	 */
	ImTextureID IMGUI_API GetTextureHandle(nap::Texture2D& texture);

	/**
	 * Displays all members of an rtti defined enumeration type inside a combo box.
	 * Call asserts if the provided type is not an enumeration object.
	 *  
	 * ~~~~~{.cpp}
	 *	if(ImGui::Combo("Type Selection", &item, RTTI_OF(nap::ETweenEasing));
	 *	{
	 *		...
	 *	}
	 * ~~~~~
	 *
	 * @param label name of the combo box
	 * @param current_item current selected item
	 * @param enum_type the enum type to display
	 * @return if selection changed
	 */
	bool IMGUI_API Combo(const char* label, int* current_item, nap::rtti::TypeInfo enum_type);
}