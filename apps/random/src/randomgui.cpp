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
		ImGui::SetNextWindowPos(ImVec2(guiWindowPadding, guiWindowPadding));
		ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, static_cast<float>(mApp.windowSize.y) - 2.0f * guiWindowPadding));
		ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::Text("left mouse button to rotate, right mouse button to zoom");
		ImGui::ListBox("Lighting Mode", &mCurrentMode, mModes, IM_ARRAYSIZE(mModes));

		switch (mCurrentMode)
		{
		case 0:
			showSunControls();
			break;
		case 1:
			showVideoControls();
			break;
		case 2:
			showStaticControls();
			break;
		default:
			break;
		}

		showOutputControls();
		ImGui::End();
	}

	void RandomGui::draw()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}

	void RandomGui::showSunControls()
	{
		if (ImGui::CollapsingHeader("Clouds", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Noise Speed", &mApp.mShaders->mNoiseSpeed, 0.0f, mApp.mShaders->mNoiseSpeedMax);
			ImGui::SliderFloat("Wind Speed", &mApp.mShaders->mWindSpeed, 0.0f, mApp.mShaders->mWindSpeedMax);
			ImGui::SliderFloat("Wind Direction", mApp.mShaders->pCloudsRotation, 0.0f, 360.0f);
			ImGui::SliderFloat("Contrast", mApp.mShaders->pCloudsContrast, 0.0f, 1.0f);
			ImGui::SliderFloat("Scale", mApp.mShaders->pCloudsScale, mApp.mShaders->mCloudsScaleMin, mApp.mShaders->mCloudsScaleMax);
			if (ImGui::Checkbox("Inverted", &mApp.mShaders->mCloudsInverted))
				mApp.mShaders->updateCloudsInverted();
		}
		if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool updateOrbit = false;
			updateOrbit = ImGui::DragFloat2("Orbit Center", mApp.mOrbit->mCenter, 0.001f, -mApp.mOrbit->mCenterRange, mApp.mOrbit->mCenterRange) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Radius", &mApp.mOrbit->mRadius, mApp.mOrbit->mRadiusMin, mApp.mOrbit->mRadiusMax) || updateOrbit;
			updateOrbit = ImGui::DragFloat2("Orbit Start / End", mApp.mOrbit->mStartEnd, 0.1f, 0.0f, 360.0f) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Progress", &mApp.mOrbit->mProgress, 0.0f, 1.0f) || updateOrbit;
			if (updateOrbit) {
				mApp.mOrbit->updateOrbit();
				mApp.mShaders->updateOrbit();
			}
			ImGui::SliderFloat("Outer Size", mApp.mShaders->pSunOuterSize, mApp.mShaders->mSunSizeMin, mApp.mShaders->mSunSizeMax);
			ImGui::SliderFloat("Inner Size", mApp.mShaders->pSunInnerSize, 0.0f, 1.0f);
			ImGui::SliderFloat("Stretch", mApp.mShaders->pSunStretch, mApp.mShaders->mSunStretchMin, mApp.mShaders->mSunStretchMax);
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

	void RandomGui::showOutputControls()
	{
		if (ImGui::CollapsingHeader("Mix", ImGuiTreeNodeFlags_DefaultOpen))
		{
			nap::RenderableMeshComponentInstance& comb_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformFloat& uBlendValue = comb_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("blendValue");
			ImGui::SliderFloat("Blend Value", &(uBlendValue.mValue), 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Light Rig", ImGuiTreeNodeFlags_DefaultOpen))
		{
			nap::ApplyCombinationComponentInstance& comb_comp = mApp.mLightRig->getComponent<ApplyCombinationComponentInstance>();
			ImGui::SliderFloat("Brightness", &(comb_comp.mBrightness), 0.0f, 1.0f);
			ImGui::SliderFloat("Influence", &(comb_comp.mInfluence), 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float avail_width = ImGui::GetContentRegionAvailWidth();
			ImGui::Text(utility::stringFormat("Framerate: %.02f", mApp.getCore().getFramerate()).c_str());
			ImGui::Image(mApp.mCombineRenderTarget->getColorTexture(), { avail_width, avail_width });
		}
	}
}
