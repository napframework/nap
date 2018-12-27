// Local Includes
#include "auragui.h"
#include "auraapp.h"

// External Includes
#include <imgui/imgui.h>
#include <nap/core.h>
#include <imguiutils.h>

namespace nap
{
	
	AuraGui::AuraGui(AuraApp& app) : mApp(app)
	{ }



	void AuraGui::init()
	{
		mColorLookupImage = mApp.mResourceManager->findObject<nap::ImageFromFile>("ColorLookupImage");
		assert(mColorLookupImage != nullptr);
	}


	void AuraGui::update(double deltaTime)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Information", nullptr, &showInfo);
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
		
		// Displays the color lookup image
		if(ImGui::CollapsingHeader("Color Lookup"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth() * mLookupSize;
			float col_ratio = static_cast<float>(mColorLookupImage->getWidth()) / static_cast<float>(mColorLookupImage->getHeight());
			ImGui::Image(*mColorLookupImage, { col_width, col_width * col_ratio });
			ImGui::SliderFloat("Preview Size", &mLookupSize, 0.0f, 1.0f);
		}
		
		ImGui::End();
	}

} 