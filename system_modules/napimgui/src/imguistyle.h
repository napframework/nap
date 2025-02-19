/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "imgui/imgui.h"

// External Includes
#include <color.h>
#include <utility/dllexport.h>

namespace nap
{
	namespace gui
	{
		inline constexpr float dpi = 96.0f;						///< Default (reference) dpi for gui elements
		inline constexpr int pointerInvalidID = -3;				///< Invalid pointer ID
		inline constexpr int pointerMouseID = -2;				///< Pointer from mouse ID
		inline constexpr int pointerTouchID = -1;				///< Pointer from touch ID

		/**
		 * All available color schemes
		 */
		enum class EColorScheme
		{
			Light		= 0,		///< Lighter color scheme
			Dark		= 1,		///< Darker color scheme (default)
			HyperDark	= 2,		///< High contrast dark color scheme
			Classic		= 3,		///< Classic color scheme
			Custom		= 4			///< Custom color scheme
		};

		/**
		 * Serializable GUI color palette
		 */
		struct NAPAPI ColorPalette
		{
			ColorPalette() = default;
			RGBColor8 mBackgroundColor = { 0x2D, 0x2D, 0x2D };		///< Property: 'BackgroundColor' Gui window background color
			RGBColor8 mDarkColor = { 0x00, 0x00, 0x00 };			///< Property: 'DarkColor' Gui dark color
			RGBColor8 mMenuColor = { 0x8D, 0x8B, 0x84 };			///< Property: 'MenuColor' Gui menu color
			RGBColor8 mFront1Color = { 0x8D, 0x8B, 0x84 };			///< Property: 'FrontColor1' Gui gradient color 1
			RGBColor8 mFront2Color = { 0xAE, 0xAC, 0xA4 };			///< Property: 'FrontColor2' Gui gradient color 2
			RGBColor8 mFront3Color = { 0xCD, 0xCD, 0xC3 };			///< Property: 'FrontColor3' Gui gradient color 3
			RGBColor8 mFront4Color = { 0xFF, 0xFF, 0xFF };			///< Property: 'FrontColor4' Gui gradient color 4 (text)
			RGBColor8 mHighlightColor1 = { 0x29, 0x58, 0xff };		///< Property: 'HighlightColor1' Special highlight color 1 (selection)
			RGBColor8 mHighlightColor2 = { 0xD6, 0xFF, 0xA3 };		///< Property: 'HighlightColor2' Special highlight color 2 (info)
			RGBColor8 mHighlightColor3 = { 0xFF, 0xEA, 0x30 };		///< Property: 'HighlightColor3' Special highlight color 3 (warning)
			RGBColor8 mHighlightColor4 = { 0xFF, 0x50, 0x50 };		///< Property: 'HighlightColor4' Special highlight color 4 (errors)
			bool mInvertIcon = false;								///< Property: 'InvertIcon' If icons should be inverted
		};

		/**
		 * Serializable GUI style options
		 */
		struct NAPAPI Style
		{
			Style() = default;
			bool mAntiAliasedLines = true;							///< Property: 'AntiAliasedLines' Enable anti-aliasing on lines/borders. Disable if you are really tight on CPU/GPU.
			bool mAntiAliasedFill = true;							///< Property: 'AntiAliasedFill' Enable anti-aliasing on filled shapes (rounded rectangles, circles, etc.)
			glm::vec2 mWindowPadding = { 10.0, 10.0f };				///< Property: 'WindowPadding' Padding within a window.
			float mWindowRounding = 0.0f;							///< Property: 'WindowRounding' Radius of window corners rounding. Set to 0.0f to have rectangular windows.
			glm::vec2 mFramePadding = { 5.0f, 5.0f };				///< Property: 'FramePadding' Padding within a framed rectangle (used by most widgets).
			float mFrameRounding = 0.0f;							///< Property: 'FrameRounding' Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
			glm::vec2 mItemSpacing = { 12.0f, 6.0f };				///< Property: 'ItemSpacing' Horizontal and vertical spacing between widgets/lines.
			glm::vec2 mItemInnerSpacing = { 8.0f, 6.0f };			///< Property: 'ItemInnerSpacing' Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
			float mIndentSpacing = 25.0f;							///< Property: 'IndentSpacing' Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
			float mScrollbarSize = 13.0f;							///< Property: 'ScrollbarSize' Width of the vertical scrollbar, Height of the horizontal scrollbar.
			float mScrollbarRounding = 0.0f;						///< Property: 'ScrollbarRounding' Radius of grab corners for scrollbar.
			float mGrabMinSize = 5.0f;								///< Property: 'GrabMinSize' Minimum width/height of a grab box for slider/scrollbar.
			float mGrabRounding = 0.0f;								///< Property: 'GrabRounding' Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
			float mWindowBorderSize = 0.0f;							///< Property: 'WindowBorderSize' Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
			float mPopupRounding = 0.0f;							///< Property: 'PopupRounding' Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
			float mChildRounding = 0.0f;							///< Property: 'ChildRounding' Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
			glm::vec2 mWindowTitleAlign = { 0.5f, 0.5f };			///< Property: 'WindowTitleAlign' Alignment for title bar text. Defaults to (0.5f,0.5f) for vertically & horizontally centered.
			float mPopupBorderSize = 0.0f;							///< Property: 'PopupBorderSize' Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
			float mTabRounding = 0.0f;								///< Property: 'TabRounding' Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
			glm::vec2 mTouchExtraPadding = { 0.0f, 0.0f };			///< Property: 'TouchExtraPadding' Expand reactive bounding box for touch-based system where touch position is not accurate enough. We don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
		};

		/**
		 * Returns the color palette for the given color scheme.
		 * Returns a nullptr if the scheme is 'Custom' and no custom color scheme is registered.
		 * @return color palette for given color scheme
		 */
		NAPAPI const ColorPalette* getPalette(EColorScheme colorScheme);

		/**
		 * Register a custom color scheme.
		 * When called multiple times, previous entries are overridden. 
		 * @param palette the custom color palette
		 * @return the color palette in the map
		 */
		NAPAPI ColorPalette& registerCustomPalette(const gui::ColorPalette& palette);

		/**
		 * Helper function, applies a color scheme to the given ImGUI style
		 * @param palette the color palette to apply
		 * @param ioStyle the style to apply color palette to
		 */
		NAPAPI void applyPalette(const gui::ColorPalette& palette, ImGuiStyle& ioStyle);

		/**
		 * Helper function, creates an ImGUI compatible style from the given configurable palette and style options
		 * @param palette the color palette to use
		 * @param style the style to use
		 * @return new ImGui compatible style
		 */
		NAPAPI std::unique_ptr<ImGuiStyle> createStyle(const gui::ColorPalette& palette, const gui::Style& style);
	}
}
