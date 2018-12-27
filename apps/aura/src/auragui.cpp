// Local Includes
#include "auragui.h"
#include "auraapp.h"

// External Includes
#include <imgui/imgui.h>
#include <nap/core.h>

namespace nap
{
	
	AuraGui::AuraGui(AuraApp& app) : mApp(app)
	{ }



	void AuraGui::init()
	{

	}


	void AuraGui::update(double deltaTime)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Controls", nullptr, &showInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showInfo)
			showInfoWindow();
	}

	void AuraGui::draw()
	{
		mApp.mGUIService->draw();
	}


	void AuraGui::showInfoWindow()
	{
		// Draw some gui elements
		ImGui::Begin("Info");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", mApp.getCore().getFramerate()).c_str());
		ImGui::End();
	}

} 