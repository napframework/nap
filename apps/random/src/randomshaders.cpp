#include "randomshaders.h"
#include "randomapp.h"

#include <mathutils.h>

namespace nap
{
	RandomShaders::RandomShaders(RandomApp& app) : mApp(app)
	{
		// Store sun-clouds uniform value pointers
		nap::RenderableMeshComponentInstance& sun_clouds = mApp.mSunClouds->getComponent<nap::RenderableMeshComponentInstance>();
		pSunCloudsOffset = &sun_clouds.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset").mValue;
		pSunCloudsRotation = &sun_clouds.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
		pSunCloudsContrast = &sun_clouds.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast").mValue;
		pSunCloudsScale = &sun_clouds.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uScale").mValue;
		pSunCloudsInverted = &sun_clouds.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInverted").mValue;

		// Store sun-glare uniform value pointers
		nap::RenderableMeshComponentInstance& sun_glare = mApp.mSunGlare->getComponent<nap::RenderableMeshComponentInstance>();
		pSunGlareOrbitCenter = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter").mValue;
		pSunGlareOrbitAngle = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle").mValue;
		pSunGlareOrbitRadius = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius").mValue;
		pSunGlareOuterSize = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize").mValue;
		pSunGlareInnerSize = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize").mValue;
		pSunGlareStretch = &sun_glare.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch").mValue;

		// Call an intial update to apply properties
		updateSunGlareOrbit();
	}

	void RandomShaders::update(double deltaTime)
	{
		// Update lighting mode transition if active
		if (mLightingModeTransitionActive)
			updateLightingModeTransition(deltaTime);

		// Update clouds shader
		updateCloudsShader(deltaTime);

		// Update camera location
		updateCameraLocation();
	}

	void RandomShaders::updateCloudsShader(double deltaTime)
	{
		// Apply wind and noise forces to cloud shader
		float windDirectionRad = nap::math::radians(*pSunCloudsRotation);
		float windDistance = mSunCloudsWindSpeed * (float)deltaTime;
		pSunCloudsOffset->x += cos(windDirectionRad) * windDistance;
		pSunCloudsOffset->y += sin(windDirectionRad) * windDistance;
		pSunCloudsOffset->z += mSunCloudsNoiseSpeed * (float)deltaTime;
	}

	void RandomShaders::updateCameraLocation()
	{
		// Retrieve camera location
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

	void RandomShaders::startLightingModeTransition(nap::Texture2D& oldTexture, nap::Texture2D& newTexture)
	{
		// Set lighting mode transition properties
		mLightingModeTransitionActive = true;
		mLightingModeTransitionVelocity = 0.0f;

		// Set combination plane uniform values at transition start
		nap::RenderableMeshComponentInstance& combination_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uBlendValue = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBlendValue");
		nap::UniformTexture2D& uTextureOne = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureOne");
		nap::UniformTexture2D& uTextureTwo = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureTwo");
		uBlendValue.setValue(0.0f);
		uTextureOne.setTexture(oldTexture);
		uTextureTwo.setTexture(newTexture);
	}

	void RandomShaders::updateLightingModeTransition(double deltaTime)
	{
		// Smoothly increment the blend uniform value in the combination plane
		nap::RenderableMeshComponentInstance& combination_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uBlendValue = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBlendValue");
		nap::math::smooth(uBlendValue.mValue, 1.0f, mLightingModeTransitionVelocity, static_cast<float>(deltaTime), 0.5f, 1000.0f);

		// Stop the updating when the transition is complete
		if (uBlendValue.mValue >= 1.0f)
		{
			mLightingModeTransitionActive = false;
			mApp.resetOldLightingMode();
		}
	}

	void RandomShaders::updateSunGlareOrbit()
	{
		pSunGlareOrbitCenter->x = mApp.mOrbit->mCenter[0];
		pSunGlareOrbitCenter->y = mApp.mOrbit->mCenter[1];
		*pSunGlareOrbitAngle = mApp.mOrbit->getAngle();
		*pSunGlareOrbitRadius = mApp.mOrbit->mRadius;
	}

	void RandomShaders::updateSunCloudsInverted()
	{
		*pSunCloudsInverted = mSunCloudsInverted ? 1.0f : 0.0f;
	}
}
