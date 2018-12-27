// Local Includes
#include "auragui.h"
#include "auraapp.h"

// External Includes
#include <imgui/imgui.h>
#include <nap/core.h>
#include <imguiutils.h>
#include <lasercontrolcomponent.h>
#include <laseroutputcomponent.h>

namespace nap
{
	
	AuraGui::AuraGui(AuraApp& app) : mApp(app)
	{ }



	void AuraGui::init()
	{
		mColorLookupImage = mApp.mResourceManager->findObject<nap::ImageFromFile>("ColorLookupImage");
		assert(mColorLookupImage != nullptr);
		mLedOnImage = mApp.mResourceManager->findObject<nap::ImageFromFile>("LedOnImage");
		assert(mLedOnImage != nullptr);
		mLedOffImage = mApp.mResourceManager->findObject<nap::ImageFromFile>("LedOffImage");
		assert(mLedOffImage != nullptr);
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
		
		if (ImGui::CollapsingHeader("Laser Status"))
		{
			LaserControlInstanceComponent& mController = mApp.mLaserController->getComponent<LaserControlInstanceComponent>();
			const std::unordered_set<int>& ids = mController.getLaserIDs();
			for (const auto& id : ids)
			{
				nap::EntityInstance* laser_entity = mController.getLaserEntity(id);
				assert(laser_entity != nullptr);
				LaserOutputComponentInstance& laser_output = laser_entity->getChildren()[1]->getComponent<nap::LaserOutputComponentInstance>();
				ImGui::TextColored(clr, "Laser: ");
				ImGui::SameLine();
				ImGui::Text(utility::stringFormat("%d", id).c_str());
				ImGui::SameLine();
				ImGui::Image(laser_output.isConnected() ? *mLedOnImage : *mLedOffImage , { 16, 16 });
				std::string status = "Ready";
				switch (laser_output.getStatus())
				{
				case EtherDreamInterface::EStatus::BUSY:
					status = "Writing Frame";
					break;
				case EtherDreamInterface::EStatus::ERROR:
					status = "Read / Write Error, check log!";
					break;
				case EtherDreamInterface::EStatus::READY:
					status = "Ready";
					break;
				}
				ImGui::SameLine();
				ImGui::TextColored(clr, "DAC: ");
				ImGui::SameLine();
				ImGui::Text(laser_output.getDacName().c_str());
				ImGui::SameLine();
				ImGui::TextColored(clr, "Status: ");
				ImGui::SameLine();
				ImGui::Text(status.c_str());
			}
		}

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