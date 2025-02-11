/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "imguistyle.h"

RTTI_BEGIN_ENUM(nap::gui::EColorScheme)
	RTTI_ENUM_VALUE(nap::gui::EColorScheme::Light,		"Light"),
	RTTI_ENUM_VALUE(nap::gui::EColorScheme::Dark,		"Dark"),
	RTTI_ENUM_VALUE(nap::gui::EColorScheme::HyperDark,	"HyperDark"),
	RTTI_ENUM_VALUE(nap::gui::EColorScheme::Classic,	"Classic"),
	RTTI_ENUM_VALUE(nap::gui::EColorScheme::Custom,		"Custom")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::gui::ColorPalette)
	RTTI_PROPERTY("BackgroundColor",	&nap::gui::ColorPalette::mBackgroundColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DarkColor",			&nap::gui::ColorPalette::mDarkColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MenuColor",			&nap::gui::ColorPalette::mMenuColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor1",		&nap::gui::ColorPalette::mFront1Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor2",		&nap::gui::ColorPalette::mFront2Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor3",		&nap::gui::ColorPalette::mFront3Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor4",		&nap::gui::ColorPalette::mFront4Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighlightColor1",	&nap::gui::ColorPalette::mHighlightColor1,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighlightColor2",	&nap::gui::ColorPalette::mHighlightColor2,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighlightColor3",	&nap::gui::ColorPalette::mHighlightColor3,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighlightColor4",	&nap::gui::ColorPalette::mHighlightColor4,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InvertIcons",		&nap::gui::ColorPalette::mInvertIcon,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::gui::Style)
	RTTI_PROPERTY("AntiAliasedLines",	&nap::gui::Style::mAntiAliasedLines,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AntiAliasedFill",	&nap::gui::Style::mAntiAliasedFill,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WindowPadding",		&nap::gui::Style::mWindowPadding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WindowRounding",		&nap::gui::Style::mWindowRounding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FramePadding",		&nap::gui::Style::mFramePadding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameRounding",		&nap::gui::Style::mFrameRounding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ItemSpacing",		&nap::gui::Style::mItemSpacing,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ItemInnerSpacing",	&nap::gui::Style::mItemInnerSpacing,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("IndentSpacing",		&nap::gui::Style::mIndentSpacing,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ScrollbarSize",		&nap::gui::Style::mScrollbarSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ScrollbarRounding",	&nap::gui::Style::mScrollbarRounding,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GrabMinSize",		&nap::gui::Style::mGrabMinSize,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GrabRounding",		&nap::gui::Style::mGrabRounding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WindowBorderSize",	&nap::gui::Style::mWindowBorderSize,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PopupRounding",		&nap::gui::Style::mPopupRounding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ChildRounding",		&nap::gui::Style::mChildRounding,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WindowTitleAlign",	&nap::gui::Style::mWindowTitleAlign,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PopupBorderSize",	&nap::gui::Style::mPopupBorderSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TabRounding",		&nap::gui::Style::mTabRounding,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TouchExtraPadding",	&nap::gui::Style::mTouchExtraPadding,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

namespace nap
{
	namespace gui
	{
		static std::unordered_map<EColorScheme, ColorPalette>& getRegisteredPalettes()
		{
			static std::unordered_map<EColorScheme, ColorPalette> scheme_map =
			{
				{EColorScheme::Light, {
					{ 0xCD, 0xCD, 0xC3 }, { 0xF5, 0xF5, 0xF3 }, { 0xa4, 0xa3, 0x9b },
					{ 0xEC, 0xFF, 0xD3 }, { 0x8D, 0x8B, 0x84 }, { 0x2D, 0x2D, 0x2D }, { 0x00, 0x00, 0x00 },
					{ 0x29, 0x58, 0xff }, { 0x8D, 0x8B, 0x84 }, { 0xFF, 0xA8, 0x00 }, { 0xFF, 0x50, 0x50 }, true}
				},
				{EColorScheme::Dark, {
					{ 0x2D, 0x2D, 0x2D }, { 0x00, 0x00, 0x00 }, { 0x4F, 0x4E, 0x4C },
					{ 0x8D, 0x8B, 0x84 }, { 0xAE, 0xAC, 0xA4 }, { 0xCD, 0xCD, 0xC3 }, { 0xFF, 0xFF, 0xFF },
					{ 0x29, 0x58, 0xff }, { 0xD6, 0xFF, 0xA3 }, { 0xFF, 0xEA, 0x30 }, { 0xFF, 0x50, 0x50 }, false}
				},
				{EColorScheme::HyperDark, {
					{ 0x00, 0x00, 0x00 }, { 0x2D, 0x2D, 0x2D }, { 0x8D, 0x8B, 0x84 },
					{ 0x8D, 0x8B, 0x84 }, { 0xAE, 0xAC, 0xA4 }, { 0xCD, 0xCD, 0xC3 }, { 0xFF, 0xFF, 0xFF },
					{ 0x29, 0x58, 0xff }, { 0xDB, 0xFF, 0x00 }, { 0xFF, 0xEA, 0x30 }, { 0xFF, 0x34, 0x7D }, false}
				},
				{EColorScheme::Classic, {
					{ 0x2D, 0x2E, 0x42 }, { 0x11, 0x14, 0x26 }, { 0x52, 0x54, 0x6A },
					{ 0x52, 0x54, 0x6A }, { 0x5D, 0x5E, 0x73 }, { 0x8B, 0x8C, 0xA0 }, { 0xFF, 0xFF, 0xFF },
					{ 0x8B, 0x8C, 0xA0 }, { 0xDB, 0xFF, 0x00 }, { 0xFF, 0xEA, 0x30 }, { 0xC8, 0x69, 0x69 }, false}
				},
			};
			return scheme_map;
		}


		ColorPalette& registerCustomPalette(const gui::ColorPalette& customPalette)
		{
			auto& palette_map = getRegisteredPalettes();
			auto it = palette_map.try_emplace(EColorScheme::Custom, customPalette);
			if (!it.second)
				it.first->second = customPalette;
			return it.first->second;
		}


		const ColorPalette* getPalette(EColorScheme colorScheme)
		{
			auto& palette_map = getRegisteredPalettes();
			auto it = palette_map.find(colorScheme);
			return it == palette_map.end() ? nullptr : &it->second;
		}


		void applyPalette(const gui::ColorPalette& palette, ImGuiStyle& ioStyle)
		{
			// Get ImGUI colors
			ImVec4 IMGUI_NAPBACK(palette.mBackgroundColor, 0.94f);
			ImVec4 IMGUI_NAPDARK(palette.mDarkColor, 0.66f);
			ImVec4 IMGUI_NAPMODA(palette.mDarkColor, 0.85f);
			ImVec4 IMGUI_NAPMENU(palette.mMenuColor, 0.66f);
			ImVec4 IMGUI_NAPFRO1(palette.mFront1Color, 1.0f);
			ImVec4 IMGUI_NAPFRO2(palette.mFront2Color, 1.0f);
			ImVec4 IMGUI_NAPFRO3(palette.mFront3Color, 1.0f);
			ImVec4 IMGUI_NAPFRO4(palette.mFront4Color, 1.0f);
			ImVec4 IMGUI_NAPHIG1(palette.mHighlightColor1, 1.0f);
			ImVec4 IMGUI_NAPHIG2(palette.mHighlightColor2, 1.0f);
			ImVec4 IMGUI_NAPHIG3(palette.mHighlightColor3, 1.0f);

			// Apply colors
			ioStyle.Colors[ImGuiCol_Text] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_TextDisabled] = IMGUI_NAPFRO2;
			ioStyle.Colors[ImGuiCol_WindowBg] = IMGUI_NAPBACK;
			ioStyle.Colors[ImGuiCol_ChildBg] = IMGUI_NAPBACK;
			ioStyle.Colors[ImGuiCol_PopupBg] = IMGUI_NAPBACK;
			ioStyle.Colors[ImGuiCol_Border] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_BorderShadow] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_FrameBg] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_FrameBgHovered] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_FrameBgActive] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_TitleBg] = IMGUI_NAPMENU;
			ioStyle.Colors[ImGuiCol_TitleBgCollapsed] = IMGUI_NAPMENU;
			ioStyle.Colors[ImGuiCol_TitleBgActive] = IMGUI_NAPFRO2;
			ioStyle.Colors[ImGuiCol_MenuBarBg] = IMGUI_NAPMENU;
			ioStyle.Colors[ImGuiCol_ScrollbarBg] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_ScrollbarGrab] = IMGUI_NAPMENU;
			ioStyle.Colors[ImGuiCol_ScrollbarGrabHovered] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_ScrollbarGrabActive] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_CheckMark] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_SliderGrab] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_SliderGrabActive] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_Button] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_ButtonHovered] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_ButtonActive] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_Header] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_HeaderHovered] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_HeaderActive] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_ResizeGrip] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_ResizeGripHovered] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_ResizeGripActive] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_Tab] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_TabHovered] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_TabActive] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_TabUnfocused] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_TabUnfocusedActive] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_PlotLines] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_PlotLinesHovered] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_PlotHistogram] = IMGUI_NAPFRO3;
			ioStyle.Colors[ImGuiCol_PlotHistogramHovered] = IMGUI_NAPHIG1;
			ioStyle.Colors[ImGuiCol_TextSelectedBg] = IMGUI_NAPFRO1;
			ioStyle.Colors[ImGuiCol_ModalWindowDimBg] = IMGUI_NAPMODA;
			ioStyle.Colors[ImGuiCol_Separator] = IMGUI_NAPDARK;
			ioStyle.Colors[ImGuiCol_SeparatorHovered] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_SeparatorActive] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_NavHighlight] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_NavWindowingHighlight] = IMGUI_NAPFRO4;
			ioStyle.Colors[ImGuiCol_NavWindowingDimBg] = IMGUI_NAPMODA;
			ioStyle.Colors[ImGuiCol_DragDropTarget] = IMGUI_NAPHIG1;
		}


		std::unique_ptr<ImGuiStyle> createStyle(const gui::ColorPalette& palette, const gui::Style& style)
		{
			// Create imgui style
			std::unique_ptr<ImGuiStyle> gui_style = std::make_unique<ImGuiStyle>();

			// Apply style settings
			gui_style->AntiAliasedFill = style.mAntiAliasedFill;
			gui_style->AntiAliasedLines = style.mAntiAliasedLines;
			gui_style->WindowPadding = { style.mWindowPadding.x, style.mWindowPadding.y };
			gui_style->WindowRounding = style.mWindowRounding;
			gui_style->FramePadding = { style.mFramePadding.x, style.mFramePadding.y };
			gui_style->FrameRounding = style.mFrameRounding;
			gui_style->ItemSpacing = { style.mItemSpacing.x, style.mItemSpacing.y };
			gui_style->ItemInnerSpacing = { style.mItemInnerSpacing.x, style.mItemInnerSpacing.y };
			gui_style->IndentSpacing = style.mIndentSpacing;
			gui_style->ScrollbarSize = style.mScrollbarSize;
			gui_style->ScrollbarRounding = style.mScrollbarRounding;
			gui_style->GrabMinSize = style.mGrabMinSize;
			gui_style->GrabRounding = style.mGrabRounding;
			gui_style->WindowBorderSize = style.mWindowBorderSize;
			gui_style->PopupRounding = style.mPopupRounding;
			gui_style->ChildRounding = style.mChildRounding;
			gui_style->WindowTitleAlign = { style.mWindowTitleAlign.x, style.mWindowTitleAlign.y };
			gui_style->PopupBorderSize = style.mPopupBorderSize;
			gui_style->TabRounding = style.mTabRounding;
			gui_style->TouchExtraPadding = { style.mTouchExtraPadding.x, style.mTouchExtraPadding.y };

			// Apply color palette
			applyPalette(palette, *gui_style);
			return gui_style;
		}
	}
}
