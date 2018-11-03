#include "randomgui.h"
#include "randomapp.h"

#include <imgui/imgui.h>
#include <imguiutils.h>
#include <selectvideocomponent.h>
#include <applycombinationcomponent.h>

namespace nap
{
	RandomGui::RandomGui(RandomApp& app) : mApp(app)
	{

	}

	void RandomGui::update(double deltaTime)
	{
		showContentControls();
		showOutputControls();
	}

	void RandomGui::draw()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}

	void RandomGui::showContentControls()
	{
		ImGui::SetNextWindowPos(ImVec2(guiWindowPadding, guiWindowPadding));
		ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, static_cast<float>(mApp.windowSize.y) - 2.0f * guiWindowPadding));
		ImGui::Begin("Content Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::ListBox("Lighting Mode", &mApp.mLightingMode, mApp.mLightingModes, IM_ARRAYSIZE(mApp.mLightingModes));

		switch (static_cast<LightingModes>(mApp.mLightingMode))
		{
		case LightingModes::Sun:
			showSunControls();
			break;
		case LightingModes::Video:
			showVideoControls();
			break;
		case LightingModes::Static:
			showStaticControls();
			break;
		default:
			break;
		}

		float avail_width = ImGui::GetContentRegionAvailWidth();
		ImGui::Spacing();
		ImGui::Text(utility::stringFormat("Framerate: %.02f", mApp.getCore().getFramerate()).c_str());
		ImGui::Image(mApp.mCombineRenderTarget->getColorTexture(), { avail_width, avail_width });
		ImGui::End();
	}

	void RandomGui::showOutputControls()
	{
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(mApp.windowSize.x) - guiWindowWidth - guiWindowPadding, guiWindowPadding));
		ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, static_cast<float>(mApp.windowSize.y) - 2.0f * guiWindowPadding));
		ImGui::Begin("Output Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		nap::ApplyCombinationComponentInstance& comb_comp = mApp.mLightRig->getComponent<ApplyCombinationComponentInstance>();
		ImGui::SliderFloat("Brightness", &(comb_comp.mBrightness), 0.0f, 1.0f);
		ImGui::SliderFloat("Influence", &(comb_comp.mInfluence), 0.0f, 1.0f);

		ImGui::End();
	}

	void RandomGui::showSunControls()
	{
		if (ImGui::CollapsingHeader("Clouds", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Noise Speed", &mApp.mShaders->mSunCloudsNoiseSpeed, 0.0f, mApp.mShaders->mSunCloudsNoiseSpeedMax);
			ImGui::SliderFloat("Wind Speed", &mApp.mShaders->mSunCloudsWindSpeed, 0.0f, mApp.mShaders->mSunCloudsWindSpeedMax);
			ImGui::SliderFloat("Wind Direction", mApp.mShaders->pSunCloudsRotation, 0.0f, 360.0f);
			ImGui::SliderFloat("Contrast", mApp.mShaders->pSunCloudsContrast, 0.0f, 1.0f);
			ImGui::SliderFloat("Scale", mApp.mShaders->pSunCloudsScale, mApp.mShaders->mSunCloudsScaleMin, mApp.mShaders->mSunCloudsScaleMax);
			if (ImGui::Checkbox("Inverted", &mApp.mShaders->mSunCloudsInverted))
				mApp.mShaders->updateSunCloudsInverted();
		}
		if (ImGui::CollapsingHeader("Glare", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool updateOrbit = false;
			updateOrbit = ImGui::DragFloat2("Orbit Center", mApp.mOrbit->mCenter, 0.001f, -mApp.mOrbit->mCenterRange, mApp.mOrbit->mCenterRange) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Radius", &mApp.mOrbit->mRadius, mApp.mOrbit->mRadiusMin, mApp.mOrbit->mRadiusMax) || updateOrbit;
			updateOrbit = ImGui::DragFloat2("Orbit Start / End", mApp.mOrbit->mStartEnd, 0.1f, 0.0f, 360.0f) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Progress", &mApp.mOrbit->mProgress, 0.0f, 1.0f) || updateOrbit;
			if (updateOrbit) {
				mApp.mOrbit->updateOrbit();
				mApp.mShaders->updateSunGlareOrbit();
			}
			ImGui::SliderFloat("Outer Size", mApp.mShaders->pSunGlareOuterSize, mApp.mShaders->mSunGlareSizeMin, mApp.mShaders->mSunGlareSizeMax);
			ImGui::SliderFloat("Inner Size", mApp.mShaders->pSunGlareInnerSize, 0.0f, 1.0f);
			ImGui::SliderFloat("Stretch", mApp.mShaders->pSunGlareStretch, mApp.mShaders->mSunGlareStretchMin, mApp.mShaders->mSunGlareStretchMax);
		}
	}

	void RandomGui::showVideoControls()
	{
		if (ImGui::CollapsingHeader("Video", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SelectVideoComponentInstance& video_comp = mApp.mVideo->getComponent<SelectVideoComponentInstance>();
			int idx = video_comp.getIndex();
			if (ImGui::SliderInt("Selection", &idx, 0, video_comp.getCount() - 1))
				video_comp.selectVideo(idx);
		}
	}

	void RandomGui::showStaticControls()
	{

	}
}
