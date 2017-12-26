// Local Includes
#include "imguiservice.h"
#include "imgui/imgui_impl_sdl_gl3.h"

// External Includes
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/core.h>
#include <color.h>

RTTI_DEFINE_CLASS(nap::IMGuiService)

namespace nap
{

	static void applyStyle()
	{
		// Get colors
		static const RGBColorFloat NAPDARK = RGBColor8(0x11, 0x14, 0x26).convert<RGBColorFloat>();
		static const RGBColorFloat NAPBACK = RGBColor8(0x2D, 0x2E, 0x42).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO1 = RGBColor8(0x52, 0x54, 0x6A).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO2 = RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO3 = RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>();
		static const RGBColorFloat NAPHIGH = RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>();

		const ImVec4 IMGUI_NAPDARK(NAPDARK.getRed(), NAPDARK.getGreen(), NAPDARK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPBACK(NAPBACK.getRed(), NAPBACK.getGreen(), NAPBACK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO1(NAPFRO1.getRed(), NAPFRO1.getGreen(), NAPFRO1.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO2(NAPFRO2.getRed(), NAPFRO2.getGreen(), NAPFRO2.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO3(NAPFRO3.getRed(), NAPFRO3.getGreen(), NAPFRO3.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPHIGH(NAPHIGH.getRed(), NAPHIGH.getGreen(), NAPHIGH.getBlue(), 1.0f);

		// Apply style
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding = ImVec2(15, 15);
		style.WindowRounding = 3.0f;
		style.FramePadding = ImVec2(5, 5);
		style.FrameRounding = 2.0f;
		style.ItemSpacing = ImVec2(12, 8);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 25.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 7.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 1.0f;

		style.Colors[ImGuiCol_Text]						= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_TextDisabled]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_WindowBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ChildWindowBg]			= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_PopupBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_Border]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_BorderShadow]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBg]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgHovered]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBgActive]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBg]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgCollapsed]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgActive]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_MenuBarBg]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarBg]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarGrab]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ScrollbarGrabHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ScrollbarGrabActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ComboBg]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_CheckMark]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SliderGrab]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SliderGrabActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Button]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ButtonHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ButtonActive]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_Header]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_HeaderHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_HeaderActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_Column]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ColumnHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ColumnActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ResizeGrip]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ResizeGripHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ResizeGripActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CloseButton]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_CloseButtonHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CloseButtonActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotLines]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_PlotLinesHovered]			= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_PlotHistogram]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotHistogramHovered]		= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_TextSelectedBg]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDarkening]		= IMGUI_NAPBACK;
	}


	void IMGuiService::render()
	{
		ImGui::Render();
	}


	bool IMGuiService::init(utility::ErrorState& error)
	{
		// Get our renderer
		mRenderer = getCore().getService<nap::RenderService>();
		assert(mRenderer != nullptr);
		
		// initialize imgui, only primary window supported for now
		if (!error.check(ImGui_ImplSdlGL3_Init(mRenderer->getPrimaryWindow().getNativeWindow()), "Unable to initialize ImGui"))
			return false;

		// Create all objects when main context is valid
		if (!error.check(ImGui_ImplSdlGL3_CreateDeviceObjects(), "Unable to create ImGui devices"))
			return false;

		// Apply color palette
		applyStyle();

		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	void IMGuiService::update(double deltaTime)
	{
		mRenderer->getPrimaryWindow().makeCurrent();	
		ImGui_ImplSdlGL3_NewFrame(mRenderer->getPrimaryWindow().getNativeWindow());
	};


	void IMGuiService::shutdown()
	{
		ImGui_ImplSdlGL3_Shutdown();
	}
}