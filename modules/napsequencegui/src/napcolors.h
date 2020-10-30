/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <imgui/imgui.h>

namespace nap
{
	namespace guicolors
	{
		const ImU32 red = 4285098440;
		const ImU32 black = 4280685585;
		const ImU32 white = 4288711819;
		const ImU32 lightGrey = 4285750877;
		const ImU32 darkGrey = 4285158482;
		const ImU32 darkerGrey = ImGui::ColorConvertFloat4ToU32(ImVec4(0.160784319f, 0.164705887f, 0.207843155f, 1));

		const ImU32 curvecolors[4] =
		{
			red,
			ImGui::ColorConvertFloat4ToU32(ImVec4(0, 1, 0, 1)),
			ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 1, 1)),
			ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 0, 1))
		};
	}
}
