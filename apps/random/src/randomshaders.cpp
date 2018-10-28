#include "randomshaders.h"
#include "randomapp.h"

#include <mathutils.h>

namespace nap
{
	RandomShaders::RandomShaders(RandomApp& app) : mApp(app)
	{
		// Store clouds uniform value pointers
		nap::RenderableMeshComponentInstance& clouds_plane = mApp.mClouds->getComponent<nap::RenderableMeshComponentInstance>();
		pCloudsOffset = &clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset").mValue;
		pCloudsRotation = &clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
		pCloudsContrast = &clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast").mValue;
		pCloudsScale = &clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uScale").mValue;
		pCloudsInverted = &clouds_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInverted").mValue;

		// Store sun uniform value pointers
		nap::RenderableMeshComponentInstance& sun_plane = mApp.mSun->getComponent<nap::RenderableMeshComponentInstance>();
		pSunOrbitCenter = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter").mValue;
		pSunOrbitAngle = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle").mValue;
		pSunOrbitRadius = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius").mValue;
		pSunOuterSize = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize").mValue;
		pSunInnerSize = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize").mValue;
		pSunStretch = &sun_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch").mValue;

		// Call an intial update to apply properties
		updateOrbit();
	}

	void RandomShaders::update(double deltaTime)
	{
		// Update clouds shader
		float windDirectionRad = nap::math::radians(*pCloudsRotation);
		float windDistance = mWindSpeed * (float)deltaTime;
		pCloudsOffset->x += cos(windDirectionRad) * windDistance;
		pCloudsOffset->y += sin(windDirectionRad) * windDistance;
		pCloudsOffset->z += mNoiseSpeed * (float)deltaTime;

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

	void RandomShaders::updateOrbit()
	{
		pSunOrbitCenter->x = mApp.mOrbit->mCenter[0];
		pSunOrbitCenter->y = mApp.mOrbit->mCenter[1];
		*pSunOrbitAngle = mApp.mOrbit->getAngle();
		*pSunOrbitRadius = mApp.mOrbit->mRadius;
	}

	void RandomShaders::updateCloudsInverted()
	{
		*pCloudsInverted = mCloudsInverted ? 1.0f : 0.0f;
	}
}
