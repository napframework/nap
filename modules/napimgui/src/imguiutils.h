#pragma once

// Local Includes
#include "imgui/imgui.h"

// External Includes
#include <texture2d.h>
#include <utility/dllexport.h>

/**
 * This file contains NAP overrides for popular IMGui functions
 * These utility functions allow you to use common NAP objects in conjunction with IMGui
 * All the functions in this file follow the IMGui style and naming conventions
 */
namespace ImGui
{
	/**
	 * Displays a NAP 2D texture as an IMGUI image
	 * @param size: display size of the texture
	 * @param: uv0 the min uv coordinates
	 * @param: uv1 the max uv coordinates
	 * @param: color used to tint the displayed texture
	 * @param: color of the border of the image
	 */
	void IMGUI_API Image(nap::Texture2D& texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}