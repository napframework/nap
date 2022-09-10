#include "imguistylesettings.h"

#include <nap/core.h>
#include <imguiservice.h>

RTTI_BEGIN_ENUM(nap::gui::StyleVar)
	RTTI_ENUM_VALUE(ImGuiStyleVar_Alpha, "Alpha"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_WindowRounding, "WindowRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_WindowBorderSize, "WindowBorderSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ChildRounding, "ChildRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ChildBorderSize, "ChildBorderSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_PopupRounding, "PopupRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_PopupBorderSize, "PopupBorderSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_FrameRounding, "FrameRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_FrameBorderSize, "FrameBorderSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_IndentSpacing, "IndentSpacing"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ScrollbarSize, "ScrollbarSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ScrollbarRounding, "ScrollbarRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_GrabMinSize, "GrabMinSize"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_GrabRounding, "GrabRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_TabRounding, "TabRounding"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_WindowPadding, "WindowPadding (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_WindowMinSize, "WindowMinSize (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_WindowTitleAlign, "WindowTitleAlign (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_FramePadding, "FramePadding (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ItemSpacing, "ItemSpacing (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ItemInnerSpacing, "ItemInnerSpacing (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_ButtonTextAlign, "ButtonTextAlign (size)"),
	RTTI_ENUM_VALUE(ImGuiStyleVar_SelectableTextAlign, "SelectableTextAlign (size)")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::gui::ColorStyleVar)
	RTTI_ENUM_VALUE(ImGuiCol_Text, "Text"),
	RTTI_ENUM_VALUE(ImGuiCol_TextDisabled, "TextDisabled"),
	RTTI_ENUM_VALUE(ImGuiCol_WindowBg, "WindowBg"),             // Background of normal windows
	RTTI_ENUM_VALUE(ImGuiCol_ChildBg, "ChildBg"),              // Background of child windows
	RTTI_ENUM_VALUE(ImGuiCol_PopupBg, "PopupBg"),              // Background of popups, menus, tooltips windows
	RTTI_ENUM_VALUE(ImGuiCol_Border, "Border"),
	RTTI_ENUM_VALUE(ImGuiCol_BorderShadow, "BorderShadow"),
	RTTI_ENUM_VALUE(ImGuiCol_FrameBg, "FrameBg"),              // Background of checkbox, radio button, plot, slider, text input
	RTTI_ENUM_VALUE(ImGuiCol_FrameBgHovered, "FrameBgHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_FrameBgActive, "FrameBgActive"),
	RTTI_ENUM_VALUE(ImGuiCol_TitleBg, "TitleBg"),
	RTTI_ENUM_VALUE(ImGuiCol_TitleBgActive, "TitleBgActive"),
	RTTI_ENUM_VALUE(ImGuiCol_TitleBgCollapsed, "TitleBgCollapsed"),
	RTTI_ENUM_VALUE(ImGuiCol_MenuBarBg, "MenuBarBg"),
	RTTI_ENUM_VALUE(ImGuiCol_ScrollbarBg, "ScrollbarBg"),
	RTTI_ENUM_VALUE(ImGuiCol_ScrollbarGrab,"ScrollbarGrab"),
	RTTI_ENUM_VALUE(ImGuiCol_ScrollbarGrabHovered, "ScrollbarGrabHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_ScrollbarGrabActive, "ScrollbarGrabActive"),
	RTTI_ENUM_VALUE(ImGuiCol_CheckMark, "CheckMark"),
	RTTI_ENUM_VALUE(ImGuiCol_SliderGrab, "SliderGrab"),
	RTTI_ENUM_VALUE(ImGuiCol_SliderGrabActive, "SliderGrabActive"),
	RTTI_ENUM_VALUE(ImGuiCol_Button, "Button"),
	RTTI_ENUM_VALUE(ImGuiCol_ButtonHovered, "ButtonHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_ButtonActive, "ButtonActive"),
	RTTI_ENUM_VALUE(ImGuiCol_Header, "Header"),               // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
	RTTI_ENUM_VALUE(ImGuiCol_HeaderHovered, "HeaderHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_HeaderActive, "HeaderActive"),
	RTTI_ENUM_VALUE(ImGuiCol_Separator, "Separator"),
	RTTI_ENUM_VALUE(ImGuiCol_SeparatorHovered, "SeparatorHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_SeparatorActive, "SeparatorActive"),
	RTTI_ENUM_VALUE(ImGuiCol_ResizeGrip, "ResizeGrip"),
	RTTI_ENUM_VALUE(ImGuiCol_ResizeGripHovered, "ResizeGripHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_ResizeGripActive, "ResizeGripActive"),
	RTTI_ENUM_VALUE(ImGuiCol_Tab, "Tab"),
	RTTI_ENUM_VALUE(ImGuiCol_TabHovered, "TabHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_TabActive, "TabActive"),
	RTTI_ENUM_VALUE(ImGuiCol_TabUnfocused, "TabUnfocused"),
	RTTI_ENUM_VALUE(ImGuiCol_TabUnfocusedActive, "TabUnfocusedActive"),
	RTTI_ENUM_VALUE(ImGuiCol_PlotLines, "PlotLines"),
	RTTI_ENUM_VALUE(ImGuiCol_PlotLinesHovered, "PlotLinesHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_PlotHistogram, "PlotHistogram"),
	RTTI_ENUM_VALUE(ImGuiCol_PlotHistogramHovered, "PlotHistogramHovered"),
	RTTI_ENUM_VALUE(ImGuiCol_TextSelectedBg, "TextSelectedBg"),
	RTTI_ENUM_VALUE(ImGuiCol_DragDropTarget, "DragDropTarget")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::gui::FloatSetting)
	RTTI_PROPERTY("Variable", &nap::gui::FloatSetting::mVariable, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Value", &nap::gui::FloatSetting::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::gui::SizeSetting)
	RTTI_PROPERTY("Variable", &nap::gui::SizeSetting::mVariable, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Value", &nap::gui::SizeSetting::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::gui::ColorSetting)
	RTTI_PROPERTY("Variable", &nap::gui::ColorSetting::mVariable, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Value", &nap::gui::ColorSetting::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::gui::StyleSettings)
    RTTI_PROPERTY("FloatSettings", &nap::gui::StyleSettings::mFloatSettings, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SizeSettings", &nap::gui::StyleSettings::mSizeSettings, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ColorSettings", &nap::gui::StyleSettings::mColorSettings, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	namespace gui
	{
    
        // TODO: re-implement scale
        static void readFloat(const std::vector<FloatSetting>& floatSettings, float& dest, StyleVar variable, bool scale = false)
        {
            auto it = std::find_if(floatSettings.begin(), floatSettings.end(), [&](auto& setting){
                return setting.mVariable == variable;
            });
            if (it != floatSettings.end())
                dest = it->mValue;
        }
    
        // TODO: re-implement scale
        static void readSize(const std::vector<SizeSetting>& sizeSettings, ImVec2& dest, StyleVar variable, bool scale = false)
        {
            auto it = std::find_if(sizeSettings.begin(), sizeSettings.end(), [&](auto& setting){
                return setting.mVariable == variable;
            });
            if (it != sizeSettings.end())
                dest = ImVec2(it->mValue.x, it->mValue.y);
        }

    
        // TODO: give scaling factor as argument here?
		void StyleSettings::apply(ImGuiStyle& imGuiStyle) const
		{
			readFloat(mFloatSettings, imGuiStyle.Alpha, StyleVar::ImGuiStyleVar_Alpha);
			readFloat(mFloatSettings, imGuiStyle.WindowRounding, StyleVar::ImGuiStyleVar_WindowRounding);
			readFloat(mFloatSettings, imGuiStyle.WindowBorderSize, StyleVar::ImGuiStyleVar_WindowBorderSize, true);
			readFloat(mFloatSettings, imGuiStyle.ChildRounding, StyleVar::ImGuiStyleVar_ChildRounding);
			readFloat(mFloatSettings, imGuiStyle.ChildBorderSize, StyleVar::ImGuiStyleVar_ChildBorderSize, true);
			readFloat(mFloatSettings, imGuiStyle.PopupRounding, StyleVar::ImGuiStyleVar_PopupRounding);
			readFloat(mFloatSettings, imGuiStyle.PopupBorderSize, StyleVar::ImGuiStyleVar_PopupBorderSize, true);
			readFloat(mFloatSettings, imGuiStyle.FrameRounding, StyleVar::ImGuiStyleVar_FrameRounding);
			readFloat(mFloatSettings, imGuiStyle.FrameBorderSize, StyleVar::ImGuiStyleVar_FrameBorderSize, true);
			readFloat(mFloatSettings, imGuiStyle.IndentSpacing, StyleVar::ImGuiStyleVar_IndentSpacing, true);
			readFloat(mFloatSettings, imGuiStyle.ScrollbarSize, StyleVar::ImGuiStyleVar_ScrollbarSize, true);
			readFloat(mFloatSettings, imGuiStyle.ScrollbarRounding, StyleVar::ImGuiStyleVar_ScrollbarRounding);
			readFloat(mFloatSettings, imGuiStyle.GrabMinSize, StyleVar::ImGuiStyleVar_GrabMinSize, true);
			readFloat(mFloatSettings, imGuiStyle.GrabRounding, StyleVar::ImGuiStyleVar_GrabRounding);
			readFloat(mFloatSettings, imGuiStyle.TabRounding, StyleVar::ImGuiStyleVar_TabRounding);
			readSize(mSizeSettings, imGuiStyle.WindowPadding, StyleVar::ImGuiStyleVar_WindowPadding, true);
			readSize(mSizeSettings, imGuiStyle.WindowMinSize, StyleVar::ImGuiStyleVar_WindowMinSize, true);
			readSize(mSizeSettings, imGuiStyle.WindowTitleAlign, StyleVar::ImGuiStyleVar_WindowTitleAlign, true);
			readSize(mSizeSettings, imGuiStyle.FramePadding, StyleVar::ImGuiStyleVar_FramePadding, true);
			readSize(mSizeSettings, imGuiStyle.ItemSpacing, StyleVar::ImGuiStyleVar_ItemSpacing, true);
			readSize(mSizeSettings, imGuiStyle.ItemInnerSpacing, StyleVar::ImGuiStyleVar_ItemInnerSpacing, true);
			readSize(mSizeSettings, imGuiStyle.ButtonTextAlign, StyleVar::ImGuiStyleVar_ButtonTextAlign, true);
			readSize(mSizeSettings, imGuiStyle.SelectableTextAlign, StyleVar::ImGuiStyleVar_SelectableTextAlign, true);

			for (auto& setting : mColorSettings)
				imGuiStyle.Colors[setting.mVariable] = ImVec4(setting.mValue[0], setting.mValue[1], setting.mValue[2], setting.mValue[3]);
		}

	} // namespace gui
} // namespace nap
