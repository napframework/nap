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
	// Define constant values
	const glm::vec2 RandomGui::uvOffset = glm::vec2(6.0f, 2.0f);
	const float RandomGui::uvScale = 136.0f;
	const float RandomGui::guiWindowWidth = 400.0f;
	const float RandomGui::guiWindowPadding = 7.0f;
	const float RandomGui::cloudsScaleMin = 0.1f;
	const float RandomGui::cloudsScaleMax = 2.0f;
	const float RandomGui::orbitCenterRange = 1.5f;
	const float RandomGui::orbitRadiusMin = 0.5f;
	const float RandomGui::orbitRadiusMax = 1.5f;
	const float RandomGui::sunSizeMin = 0.05f;
	const float RandomGui::sunSizeMax = 0.15f;
	const float RandomGui::sunStretchMin = 1.0f;
	const float RandomGui::sunStretchMax = 10.0f;

	void RandomGui::update(double deltaTime)
	{
		// Show control menu
		showControlWindow();

		// Update some shader variables
		nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uRotation = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation");
		nap::UniformVec3& uOffset = clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset");
		nap::RenderableMeshComponentInstance& sun_plane = mApp.mSun->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformVec3& uOrbitCenter = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter");
		nap::UniformFloat& uOrbitRadius = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius");
		nap::UniformFloat& uOrbitAngle = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle");
		float windDirectionRad = nap::math::radians(uRotation.mValue);
		float windDistance = mWindSpeed * (float)deltaTime;
		uOffset.mValue.x += cos(windDirectionRad) * windDistance;
		uOffset.mValue.y += sin(windDirectionRad) * windDistance;
		uOffset.mValue.z += mNoiseSpeed * (float)deltaTime;
		uOrbitAngle.setValue(getOrbitAngle());

		// Apply initial values to the orbit circle and elements
		setOrbitPosition(&(uOrbitCenter.mValue.x), &(uOrbitCenter.mValue.y));
		setOrbitPathRadius(&(uOrbitRadius.mValue));
		setOrbitSunPosition(&(uOrbitAngle.mValue), &(uOrbitRadius.mValue));
		setOrbitStartEndPosition(&(uOrbitRadius.mValue));

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
		ImGui::SetNextWindowPos(ImVec2(guiWindowPadding, guiWindowPadding));
		ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, static_cast<float>(mApp.windowSize.y) - 2.0f * guiWindowPadding));
		ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::Text("left mouse button to rotate, right mouse button to zoom");
		
		if (ImGui::CollapsingHeader("Clouds", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Noise Speed", &mNoiseSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wind Speed", &mWindSpeed, 0.0f, 1.0f);

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
			nap::UniformVec3& uOrbitCenter = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter");
			nap::UniformFloat& uOrbitRadius = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius");
			nap::UniformFloat& uOrbitAngle = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle");
			nap::UniformFloat& uOuterSize = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize");
			nap::UniformFloat& uInnerSize = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize");
			nap::UniformFloat& uStretch = sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch");
			bool updateOrbitX = ImGui::SliderFloat("Orbit Center X", &(uOrbitCenter.mValue.x), -orbitCenterRange, orbitCenterRange);
			bool updateOrbitY = ImGui::SliderFloat("Orbit Center Y", &(uOrbitCenter.mValue.y), -orbitCenterRange, orbitCenterRange);
			bool updateOrbitRadius = ImGui::SliderFloat("Orbit Radius", &(uOrbitRadius.mValue), orbitRadiusMin, orbitRadiusMax);
			bool updateOrbitStartEnd = ImGui::DragFloat2("Orbit Start / End", mOrbitStartEnd, 1.0f, 0.0f, 360.0f);
			bool updateOrbitProgress = ImGui::SliderFloat("Orbit Progress", &mOrbitProgress, 0.0f, 1.0f);
			ImGui::SliderFloat("Outer Size", &(uOuterSize.mValue), sunSizeMin, sunSizeMax);
			ImGui::SliderFloat("Inner Size", &(uInnerSize.mValue), 0.0f, 1.0f);
			ImGui::SliderFloat("Stretch", &(uStretch.mValue), sunStretchMin, sunStretchMax);

			if (updateOrbitStartEnd || updateOrbitProgress)
				uOrbitAngle.setValue(getOrbitAngle());

			if (updateOrbitX || updateOrbitY)
				setOrbitPosition(&(uOrbitCenter.mValue.x), &(uOrbitCenter.mValue.y));

			if (updateOrbitRadius)
				setOrbitPathRadius(&(uOrbitRadius.mValue));

			if (updateOrbitStartEnd || updateOrbitProgress || updateOrbitRadius)
				setOrbitSunPosition(&(uOrbitAngle.mValue), &(uOrbitRadius.mValue));

			if (updateOrbitStartEnd || updateOrbitRadius)
				setOrbitStartEndPosition(&(uOrbitRadius.mValue));
		}
		if (ImGui::CollapsingHeader("Video", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SelectVideoComponentInstance& video_comp = mApp.mVideo->getComponent<SelectVideoComponentInstance>();
			int idx = video_comp.getIndex();
			if (ImGui::SliderInt("Selection", &idx, 0, video_comp.getCount()-1))
				video_comp.selectVideo(idx);
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

	float RandomGui::getOrbitAngle() {
		return mOrbitStartEnd[0] - mOrbitProgress * (mOrbitStartEnd[0] - mOrbitStartEnd[1]);
	}

	void RandomGui::setOrbitPosition(float *x, float *z)
	{
		nap::TransformComponentInstance& orbit_transform = mApp.mOrbit->getComponent<nap::TransformComponentInstance>();
		glm::vec3 translate = orbit_transform.getTranslate();
		translate.x = uvOffset.x + *x * uvScale;
		translate.z = uvOffset.y + *z * -uvScale;
		orbit_transform.setTranslate(translate);
	}

	void RandomGui::setOrbitPathRadius(float *radius)
	{
		nap::TransformComponentInstance& orbit_path_transform = mApp.mOrbitPath->getComponent<nap::TransformComponentInstance>();
		orbit_path_transform.setUniformScale(*radius * uvScale);
	}

	void RandomGui::setOrbitSunPosition(float *angle, float *radius)
	{
		nap::TransformComponentInstance& orbit_sun_transform = mApp.mOrbitSun->getComponent<nap::TransformComponentInstance>();
		glm::vec3 translate = glm::vec3();
		translate.x = cos(nap::math::radians(-*angle)) * *radius * uvScale;
		translate.y = sin(nap::math::radians(-*angle)) * *radius * uvScale;
		orbit_sun_transform.setTranslate(translate);
	}

	void RandomGui::setOrbitStartEndPosition(float *radius)
	{
		nap::TransformComponentInstance& orbit_start_transform = mApp.mOrbitStart->getComponent<nap::TransformComponentInstance>();
		nap::TransformComponentInstance& orbit_end_transform = mApp.mOrbitEnd->getComponent<nap::TransformComponentInstance>();
		glm::vec3 start_translate = glm::vec3();
		glm::vec3 end_translate = glm::vec3();
		start_translate.x = cos(nap::math::radians(-mOrbitStartEnd[0])) * *radius * uvScale;
		start_translate.y = sin(nap::math::radians(-mOrbitStartEnd[0])) * *radius * uvScale;
		end_translate.x = cos(nap::math::radians(-mOrbitStartEnd[1])) * *radius * uvScale;
		end_translate.y = sin(nap::math::radians(-mOrbitStartEnd[1])) * *radius * uvScale;
		orbit_start_transform.setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mOrbitStartEnd[0]))));
		orbit_start_transform.setTranslate(start_translate);
		orbit_end_transform.setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mOrbitStartEnd[1]))));
		orbit_end_transform.setTranslate(end_translate);
	}
}
