// Local Includes
#include "atmosgui.h"
#include "atmosapp.h"
#include "selectmeshcomponent.h"
#include "selectimagecomponent.h"
#include "updatematerialcomponent.h"

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

		// Select mesh slider
		nap::SelectMeshComponentInstance& mesh_selector = mApp.mScanEntity->getComponent<SelectMeshComponentInstance>();
		int ci = mesh_selector.getIndex();
		if (ImGui::SliderInt("Select Mesh", &ci, 0, mesh_selector.getCount() - 1))
		{
			mesh_selector.selectMesh(ci);
		}

		// Select image 1 (tileable) slider
		nap::SelectImageComponentInstance* img_selector_tileable = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentTileable");
		assert(img_selector_tileable != nullptr);
		ci = img_selector_tileable->getIndex();
		if (ImGui::SliderInt("Select Tileable Texture", &ci, 0, img_selector_tileable->getCount() - 1))
		{
			img_selector_tileable->selectImage(ci);
		}

		// Select image 2 (stretched / single) slider
		nap::SelectImageComponentInstance* img_selector_single = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentSingle");
		assert(img_selector_single != nullptr);
		ci = img_selector_single->getIndex();
		if (ImGui::SliderInt("Select Stretch Texture", &ci, 0, img_selector_single->getCount() - 1))
		{
			img_selector_single->selectImage(ci);
		}

		// Mix Controls
		nap::UpdateMaterialComponentInstance& up_mat_comp = mApp.mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		if (ImGui::CollapsingHeader("Mix"))
		{
			ImGui::ColorEdit3("Diffuse Color", up_mat_comp.mDiffuseColor.getData());
			ImGui::SliderFloat("Texture Blend Value", &(up_mat_comp.mColorTexMix), 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse Blend Value", &(up_mat_comp.mDiffuseColorMix), 0.0f, 1.0f);
		}

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