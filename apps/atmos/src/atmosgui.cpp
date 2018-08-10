// Local Includes
#include "atmosgui.h"
#include "atmosapp.h"

// External Includes
#include <imgui/imgui.h>
#include <nap/core.h>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showControls = false;
	static bool showInfo = false;

	AtmosGui::AtmosGui(AtmosApp& app) : mApp(app)
	{

	}


	void AtmosGui::init()
	{

	}


	void AtmosGui::update()
	{
		if (mHide)
			return;

		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Controls", NULL, &showControls);
				ImGui::MenuItem("Information", NULL, &showInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Show control menu
		if (showControls)
			showControlWindow();

		if (showInfo)
			showInfoWindow();
	}
	

	void AtmosGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void AtmosGui::toggleVisibility()
	{
		mHide = !mHide;
	}


	void AtmosGui::showControlWindow()
	{
		// Resets all the tracers
		ImGui::Begin("Controls");
		ImGui::End();
	}


	void AtmosGui::showInfoWindow()
	{
		// Color used for highlights
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImVec4 float_clr_gui = { text_color[0], text_color[1], text_color[2], 1.0f };

		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		ImGui::Spacing();
		utility::getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		ImGui::TextColored(float_clr_gui, "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		ImGui::End();
	}

}