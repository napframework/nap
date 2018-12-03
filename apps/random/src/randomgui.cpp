#include "randomgui.h"
#include "randomapp.h"

// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <mathutils.h>
#include <selectvideocomponent.h>
#include <applycombinationcomponent.h>

namespace nap
{
	void RandomGui::update(double deltaTime)
	{
		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Controls", NULL, &mShowControls);
				ImGui::MenuItem("Information", NULL, &mShowInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Show control menu
		if (mShowControls) 
			showControlWindow();

		if (mShowInfo)
			showInfoWindow();

		// Update some shader variables
		nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformVec3& uOffset = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset");
		float windDirectionRad = nap::math::radians(mWindDirection);
		float windDistance = mWindSpeed * (float)deltaTime;
		uOffset.mValue.x += cos(windDirectionRad) * windDistance;
		uOffset.mValue.y += sin(windDirectionRad) * windDistance;
		uOffset.mValue.z += mNoiseSpeed * (float)deltaTime;

		// Update camera location
		TransformComponentInstance& cam_xform = mApp.mSceneCamera->getComponent<TransformComponentInstance>();
		glm::vec3 cam_pos = math::extractPosition(cam_xform.getGlobalTransform());
		nap::MaterialInstance& rig_material = mApp.mLightRig->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
		nap::UniformVec3& ucam_pos = rig_material.getOrCreateUniform<nap::UniformVec3>("cameraLocation");
		ucam_pos.setValue(cam_pos);
	}

	void RandomGui::init()
	{

	}

	void RandomGui::draw()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	RandomGui::RandomGui(RandomApp& app) : mApp(app)
	{

	}

	void RandomGui::showControlWindow()
	{
		ImGui::Begin("Controls");
		if (ImGui::CollapsingHeader("Cloud"))
		{
			ImGui::SliderFloat("Noise Speed", &mNoiseSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wind Speed", &mWindSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wind Direction", &mWindDirection, 0.0, 360.0);

			nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformFloat& uContrast = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast");
			ImGui::SliderFloat("Contrast", &(uContrast.mValue), 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Video"))
		{
			SelectVideoComponentInstance& video_comp = mApp.mVideo->getComponent<SelectVideoComponentInstance>();
			int idx = video_comp.getIndex();
			if (ImGui::SliderInt("Selection", &idx, 0, video_comp.getCount()-1))
				video_comp.selectVideo(idx);
		}
		if (ImGui::CollapsingHeader("Mix"))
		{
			nap::RenderableMeshComponentInstance& comb_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformFloat& uBlendValue = comb_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("blendValue");
			ImGui::SliderFloat("Blend Value", &(uBlendValue.mValue), 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Light Rig"))
		{
			nap::ApplyCombinationComponentInstance& comb_comp = mApp.mLightRig->getComponent<ApplyCombinationComponentInstance>();
			ImGui::SliderFloat("Brightness", &(comb_comp.mBrightness), 0.0f, 1.0f);
			ImGui::SliderFloat("Influence", &(comb_comp.mInfluence), 0.0f, 1.0f);
		}
		ImGui::End();
	}


	void RandomGui::showInfoWindow()
	{
		ImGui::Begin("Information");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", mApp.getCore().getFramerate()).c_str());
		ImGui::SliderFloat("Preview Size", &mTextureDisplaySize, 0.0f, 1.0f);
		float col_width = ImGui::GetContentRegionAvailWidth() * mTextureDisplaySize;
		if (ImGui::CollapsingHeader("Cloud Texture"))
		{
			ImGui::Image(mApp.mCloudRenderTarget->getColorTexture(),   { col_width, col_width });
		}
		if (ImGui::CollapsingHeader("Video Texture"))
		{
			ImGui::Image(mApp.mVideoRenderTarget->getColorTexture(), { col_width, col_width });
		}
		if (ImGui::CollapsingHeader("Combined Texture"))
		{
			ImGui::Image(mApp.mCombineRenderTarget->getColorTexture(), { col_width, col_width });
		}
		ImGui::End();
	}

}