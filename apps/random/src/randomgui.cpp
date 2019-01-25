#include "randomgui.h"
#include "randomapp.h"

#include <imgui/imgui.h>
#include <imguiutils.h>
#include <selectvideocomponent.h>
#include <applycombinationcomponent.h>
#include <updatematerialcomponent.h>
#include <orbitcomponent.h>
#include <lightingmodecomponent.h>

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
		LightingModeComponentInstance& lightingModeComponent = mApp.mController->getComponent<LightingModeComponentInstance>();
		ImGui::SetNextWindowPos(ImVec2(guiWindowPadding, guiWindowPadding));
		ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, static_cast<float>(mApp.windowSize.y) - 2.0f * guiWindowPadding));
		ImGui::Begin("Content Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (ImGui::ListBox("Lighting Mode", &lightingModeComponent.mLightingModeInt, lightingModeComponent.mLightingModes, IM_ARRAYSIZE(lightingModeComponent.mLightingModes)))
			lightingModeComponent.startLightingModeTransition();

		switch (lightingModeComponent.mLightingModeEnum)
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
		case LightingModes::Party:
			showPartyControls();
			break;
		case LightingModes::Sound:
			showSoundControls();
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

		if (ImGui::CollapsingHeader("Master Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			nap::ApplyCombinationComponentInstance& comb_comp = mApp.mLightRig->getComponent<ApplyCombinationComponentInstance>();
			ImGui::SliderFloat("Brightness", &(comb_comp.mBrightness), 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Group Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& control_group : mApp.mControlGroups->mControlGroups)
			{
				const char * name = control_group.mName.c_str();
				ImGui::Text(name);
				ImGui::PushID(name);
				ImGui::SliderFloat("Brightness", &(control_group.mBrightness), 0.0f, 1.0f);
				ImGui::PopID();
			}
		}		

		ImGui::End();
	}

	void RandomGui::showSunControls()
	{
		UpdateMaterialComponentInstance& updateMaterial = mApp.mController->getComponent<UpdateMaterialComponentInstance>();
		if (ImGui::CollapsingHeader("Clouds", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Cloud Temperature", updateMaterial.getSunCloudsTemperaturePtr(), 0.0, 1.0);
			ImGui::SliderFloat("Cloud Ratio", updateMaterial.getSunCloudsCloudRatioPtr(), 0.0, 1.0);
			ImGui::SliderFloat("Cloud Fill", updateMaterial.getSunCloudsCloudFillPtr(), 0.0, 1.0);
			ImGui::SliderFloat("Light Fill", updateMaterial.getSunCloudsLightFillPtr(), 0.0, 1.0);
			ImGui::SliderFloat("Noise Speed", &updateMaterial.mSunCloudsNoiseSpeed, 0.0f, updateMaterial.mSunCloudsNoiseSpeedMax);
			ImGui::SliderFloat("Wind Speed", &updateMaterial.mSunCloudsWindSpeed, 0.0f, updateMaterial.mSunCloudsWindSpeedMax);
			ImGui::SliderFloat("Wind Direction", updateMaterial.getSunCloudsRotationPtr(), 0.0f, 360.0f);
			ImGui::SliderFloat("Contrast", updateMaterial.getSunCloudsContrastPtr(), 0.0f, 1.0f);
			ImGui::SliderFloat("Scale", updateMaterial.getSunCloudsScalePtr(), updateMaterial.mSunCloudsScaleMin, updateMaterial.mSunCloudsScaleMax);
		}
		OrbitComponentInstance& orbit = mApp.mOrbit->getComponent<OrbitComponentInstance>();
		if (ImGui::CollapsingHeader("Glare", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// if we change any properties on the orbit component, make sure
			// we update its transform components and the sun-glare shader
			bool updateOrbit = false;
			ImGui::SliderFloat("Glare Temperature", updateMaterial.getSunGlareTemperaturePtr(), 0.0, 1.0);
			updateOrbit = ImGui::DragFloat2("Orbit Center", orbit.mCenter, 0.001f, -orbit.mCenterRange, orbit.mCenterRange) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Radius", &orbit.mRadius, orbit.mRadiusMin, orbit.mRadiusMax) || updateOrbit;
			updateOrbit = ImGui::DragFloat2("Orbit Start / End", orbit.mStartEnd, 0.1f, 0.0f, 360.0f) || updateOrbit;
			updateOrbit = ImGui::DragIntRange2("Orbit Hours", &orbit.mStartHour, &orbit.mEndHour, 0.2f, 0, 23) || updateOrbit;

			ImGui::Checkbox("Manual Orbit Progression", &orbit.mManualProgress);
			if (orbit.mManualProgress)
			{
				// if we manually control the orbit progression, update when we drag the slider
				updateOrbit = ImGui::SliderFloat("Orbit Progress", &orbit.mProgress, 0.0f, 1.0f) || updateOrbit;
			}
			else
			{
				// otherwise, set the orbit progress by the current time and always update
				orbit.mProgress = orbit.getProgressByTime();
				ImGui::ProgressBar(orbit.mProgress, ImVec2(-1, 0), "Orbit Progress");
				updateOrbit = true;
			}
			if (updateOrbit)
			{
				orbit.updateOrbit();
				updateMaterial.updateSunGlareOrbit();
			}
			ImGui::SliderFloat("Outer Size", updateMaterial.getSunGlareOuterSizePtr(), updateMaterial.mSunGlareSizeMin, updateMaterial.mSunGlareSizeMax);
			ImGui::SliderFloat("Inner Size", updateMaterial.getSunGlareInnerSizePtr(), 0.0f, 1.0f);
			ImGui::SliderFloat("Stretch", updateMaterial.getSunGlareStretchPtr(), updateMaterial.mSunGlareStretchMin, updateMaterial.mSunGlareStretchMax);
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
		if (ImGui::CollapsingHeader("Static", ImGuiTreeNodeFlags_DefaultOpen))
		{
			UpdateMaterialComponentInstance& updateMaterial = mApp.mController->getComponent<UpdateMaterialComponentInstance>();
			ImGui::SliderFloat("Static Temperature", updateMaterial.getStaticTemperaturePtr(), 0.0f, 1.0f);
		}
	}

	void RandomGui::showPartyControls()
	{
		UpdateMaterialComponentInstance& updateMaterial = mApp.mController->getComponent<UpdateMaterialComponentInstance>();
		if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Checkbox("Use Party Presets", &updateMaterial.mPartyPresetsActive))
				updateMaterial.startPartyPresetTransition(updateMaterial.mPartyPresetsActive ? PARTY_PRESET_CALM : PARTY_PRESET_NONE);
		}
		if (ImGui::CollapsingHeader("Music", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::SliderFloat2("Center", updateMaterial.mPartyCenter, 0.0f, 1.0f))
				updateMaterial.updatePartyCenter();
			ImGui::DragFloat("BPM", &updateMaterial.mPartyBPM, 0.25f, updateMaterial.mPartyBPMMin, updateMaterial.mPartyBPMMax);			
		}
		if (ImGui::CollapsingHeader("Waves", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("Count", updateMaterial.getPartyWaveCountPtr(), 1, updateMaterial.mPartyWaveCountMax);
			ImGui::SliderFloat("Length", updateMaterial.getPartyWaveLengthPtr(), updateMaterial.mPartyWaveLengthMin, 1.0f);
			ImGui::SliderFloat("Center", updateMaterial.getPartyWaveCenterPtr(), 0.0f, 1.0f);
			ImGui::SliderFloat("Highlight Length", updateMaterial.getPartyWaveHighlightLengthPtr(), 0.0f, 1.0f);
			ImGui::SliderFloat("Highlight Intensity", updateMaterial.getPartyWaveHighlightIntensityPtr(), 0.0f, 1.0f);
			ImGui::DragFloatRange2("Fall-off Start / End", updateMaterial.getPartyWaveFalloffStartPtr(), updateMaterial.getPartyWaveFalloffEndPtr(), 0.01f, updateMaterial.mPartyWaveFalloffMin, updateMaterial.mPartyWaveFalloffMax);
			ImGui::SliderFloat("Noise Speed", &updateMaterial.mPartyWaveNoiseSpeed, 0.0f, updateMaterial.mPartyWaveNoiseSpeedMax);
			ImGui::SliderFloat("Noise Scale", updateMaterial.getPartyWaveNoiseScalePtr(), updateMaterial.mPartyWaveNoiseScaleMin, updateMaterial.mPartyWaveNoiseScaleMax);
			ImGui::SliderFloat("Noise Influence", updateMaterial.getPartyWaveNoiseInfluencePtr(), 0.0f, updateMaterial.mPartyWaveNoiseInfluenceMax);
		}
	}

	void RandomGui::showSoundControls()
	{
		if (ImGui::CollapsingHeader("Sound", ImGuiTreeNodeFlags_DefaultOpen))
		{
			UpdateMaterialComponentInstance& updateMaterial = mApp.mController->getComponent<UpdateMaterialComponentInstance>();
			ImGui::SliderFloat("Sound Temperature", updateMaterial.getSoundTemperaturePtr(), 0.0f, 1.0f);
		}
	}
}
