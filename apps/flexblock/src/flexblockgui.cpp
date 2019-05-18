// Local Includes
#include "flexblockgui.h"
#include "flexblockapp.h"

// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <nap/core.h>
#include <utility/fileutils.h>
#include <parametergui.h>
#include <meshutils.h>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showInfo = false;
	static bool showPresetWindow = false;

	FlexblockGui::FlexblockGui(FlexblockApp& app) : 
		mApp(app),
		mParameterService(*app.getCore().getService<ParameterService>()),
		mBlockMeshInstance(app.GetBlockEntity()->getComponent<RenderableMeshComponentInstance>()),
		mVertexAttribute(mBlockMeshInstance.getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName())),
		mNormalAttribute(mBlockMeshInstance.getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getNormalName()))
	{
	}


	FlexblockGui::~FlexblockGui()
	{
	}


	void FlexblockGui::init()
	{
		// Create parameter gui
		mParameterGUI = std::make_unique<ParameterGUI>(mParameterService);

		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();
	}


	void FlexblockGui::update()
	{
		if (mHide)
			return;

		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Parameters", NULL, &showPresetWindow);
				ImGui::MenuItem("Information", NULL, &showInfo);

				std::vector<glm::vec3> data = mVertexAttribute.getData();
				float vertPos[3] = { data[0].x, data[0].y, data[0].z };

				if (ImGui::DragFloat3("Vertex Position", vertPos)) 
				{
					data[0].x = vertPos[0];
					data[0].y = vertPos[1];
					data[0].z = vertPos[2];

					data[13] = data[0];
					data[16] = data[0];

					mVertexAttribute.setData(data);

					utility::computeNormals(mBlockMeshInstance.getMeshInstance(), mVertexAttribute, mNormalAttribute);
					
					utility::ErrorState error;
					mBlockMeshInstance.getMeshInstance().update(error);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showPresetWindow)
			mParameterGUI->show(mParameterService.hasRootGroup() ? &mParameterService.getRootGroup() : nullptr);

		if (showInfo)
			showInfoWindow();
	}
	

	void FlexblockGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void FlexblockGui::toggleVisibility()
	{
		mHide = !mHide;
	}


	void FlexblockGui::showInfoWindow()
	{
		// Color used for highlights
		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		ImGui::Spacing();
		getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::TextColored(text_color, "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		ImGui::End();
	}

}