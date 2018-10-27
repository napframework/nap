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
	RandomGui::RandomGui(RandomApp& app) : mApp(app)
	{

	}

	void RandomGui::update(double deltaTime)
	{
		// Show control menu
		showControlWindow();

		// Update clouds shader
		nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uRotation = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation");
		nap::UniformVec3& uOffset = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset");
		float windDirectionRad = nap::math::radians(uRotation.mValue);
		float windDistance = mWindSpeed * (float)deltaTime;
		uOffset.mValue.x += cos(windDirectionRad) * windDistance;
		uOffset.mValue.y += sin(windDirectionRad) * windDistance;
		uOffset.mValue.z += mNoiseSpeed * (float)deltaTime;
		
		// Update sun shader
		nap::RenderableMeshComponentInstance& sun_plane = mApp.mSun->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uOrbitAngle = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle");
		uOrbitAngle.setValue(mApp.mOrbit->getAngle());

		// Update camera location
		TransformComponentInstance& cam_xform = mApp.mSceneCamera->getComponent<TransformComponentInstance>();
		glm::vec3 cam_pos = math::extractPosition(cam_xform.getGlobalTransform());

		// Get all renderable meshes under the light rig and set camera location uniform (they should all have one)
		std::vector<RenderableMeshComponentInstance*> render_meshes;
		mApp.mLightRig->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(render_meshes);
		for (auto& rmesh : render_meshes)
		{
			nap::MaterialInstance& material = rmesh->getMaterialInstance();
			material.getOrCreateUniform<nap::UniformVec3>("cameraLocation").setValue(cam_pos);
		}
	}

	void RandomGui::draw()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}

	void RandomGui::showControlWindow()
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

		ImGui::End();
	}

	void RandomGui::showSunControls()
	{
		if (ImGui::CollapsingHeader("Clouds", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Noise Speed", &mNoiseSpeed, 0.0f, 0.25f);
			ImGui::SliderFloat("Wind Speed", &mWindSpeed, 0.0f, 0.5f);

			nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformFloat& uRotation = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation");
			nap::UniformFloat& uContrast = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast");
			nap::UniformFloat& uScale = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uScale");
			ImGui::SliderFloat("Wind Direction", &(uRotation.mValue), 0.0f, 360.0f);
			ImGui::SliderFloat("Contrast", &(uContrast.mValue), 0.0f, 1.0f);
			ImGui::SliderFloat("Scale", &(uScale.mValue), cloudsScaleMin, cloudsScaleMax);
			if (ImGui::Checkbox("Inverted", &mCloudsInverted))
			{
				nap::UniformFloat& uInverted = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInverted");
				uInverted.setValue(mCloudsInverted ? 1.0f : 0.0f);
			}
		}
		if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen))
		{
			nap::RenderableMeshComponentInstance& sun_plane = mApp.mSun->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformFloat& uOuterSize = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize");
			nap::UniformFloat& uInnerSize = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize");
			nap::UniformFloat& uStretch = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch");

			bool updateOrbit = false;
			updateOrbit = ImGui::DragFloat2("Orbit Center", mApp.mOrbit->mCenter, 0.001f, -orbitCenterRange, orbitCenterRange) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Radius", &mApp.mOrbit->mRadius, orbitRadiusMin, orbitRadiusMax) || updateOrbit;
			updateOrbit = ImGui::DragFloat2("Orbit Start / End", mApp.mOrbit->mStartEnd, 0.1f, 0.0f, 360.0f) || updateOrbit;
			updateOrbit = ImGui::SliderFloat("Orbit Progress", &mApp.mOrbit->mProgress, 0.0f, 1.0f) || updateOrbit;

			ImGui::SliderFloat("Outer Size", &(uOuterSize.mValue), sunSizeMin, sunSizeMax);
			ImGui::SliderFloat("Inner Size", &(uInnerSize.mValue), 0.0f, 1.0f);
			ImGui::SliderFloat("Stretch", &(uStretch.mValue), sunStretchMin, sunStretchMax);

			if (updateOrbit) {
				// Update orbit properties of sun shader
				nap::UniformVec3& uOrbitCenter = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter");
				nap::UniformFloat& uOrbitAngle = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle");
				nap::UniformFloat& uOrbitRadius = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius");
				uOrbitCenter.mValue.x = mApp.mOrbit->mCenter[0];
				uOrbitCenter.mValue.y = mApp.mOrbit->mCenter[1];
				uOrbitAngle.setValue(mApp.mOrbit->getAngle());
				uOrbitRadius.setValue(mApp.mOrbit->mRadius);

				// update orbit scene components
				mApp.mOrbit->update();
			}
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
