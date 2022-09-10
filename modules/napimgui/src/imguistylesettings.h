#pragma once

#include <nap/resource.h>
#include <imgui/imgui.h>

namespace nap
{
    // Forward declarations
    class IMGuiService;

	namespace gui
	{
    
        using StyleVar = ImGuiStyleVar_;
        using ColorStyleVar = ImGuiCol_;

        struct FloatSetting
        {
            StyleVar mVariable; ///< Indicates what ImGui style variable is specified by this setting
            float mValue; ///< The value of the style variable
        };

        /**
         * A style setting of a type vec2 style variable, specifying a rectangular size.
         */
        struct SizeSetting
        {
            StyleVar mVariable; ///< Indicates what ImGui style variable is specified by this setting
            glm::vec2 mValue; ///< The value of the style vatiable
        };

        /**
         * A setting for a color style variable
         */
        struct ColorSetting
        {
            ColorStyleVar mVariable; ///< Indicates what ImGui color style variable is speicified by this setting
            glm::vec4 mValue; ///< The value of the color style variable
        };

    
        struct NAPAPI StyleSettings
        {
            StyleSettings() = default;
            
            std::vector<FloatSetting> mFloatSettings; ///< Property: 'FloatSettings' All style variable settings of type float
            std::vector<SizeSetting> mSizeSettings; ///< Property: 'SizeSettings' All style variable settings of type vec2, indicating a rectangle size
            std::vector<ColorSetting> mColorSettings; ///< Property: 'ColorSettings' All style variable settings specifying a color
            
            void apply(ImGuiStyle& imGuiStyle) const;
        };
    
	} // namespace gui

} // namespace nap
