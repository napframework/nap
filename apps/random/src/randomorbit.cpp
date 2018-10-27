#include "randomorbit.h"
#include "randomapp.h"

#include <mathutils.h>

namespace nap
{
	RandomOrbit::RandomOrbit(Scene& scene)
	{
		// Store scene objects
		mOrbit = scene.findEntity("Orbit");
		mOrbitPath = scene.findEntity("OrbitPath");
		mOrbitStart = scene.findEntity("OrbitStart");
		mOrbitEnd = scene.findEntity("OrbitEnd");
		mOrbitSun = scene.findEntity("OrbitSun");
		
		// Call an intial update to set properties
		update();
	}

	void RandomOrbit::update()
	{
		// Update orbit position
		nap::TransformComponentInstance& orbit_transform = mOrbit->getComponent<nap::TransformComponentInstance>();
		orbit_transform.setTranslate(glm::vec3(
			mUvOffset.x + mCenter[0] * mUvScale,
			orbit_transform.getTranslate().y,
			mUvOffset.y + mCenter[1] * -mUvScale
		));

		// Update orbit path scale
		nap::TransformComponentInstance& orbit_path_transform = mOrbitPath->getComponent<nap::TransformComponentInstance>();
		orbit_path_transform.setUniformScale(mRadius * mUvScale);

		// Update orbit start position and rotation
		nap::TransformComponentInstance& orbit_start_transform = mOrbitStart->getComponent<nap::TransformComponentInstance>();
		orbit_start_transform.setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mStartEnd[0]))));
		orbit_start_transform.setTranslate(glm::vec3(
			cos(nap::math::radians(-mStartEnd[0])) * mRadius * mUvScale,
			sin(nap::math::radians(-mStartEnd[0])) * mRadius * mUvScale,
			0.0f		
		));

		// Update orbit end position and rotation
		nap::TransformComponentInstance& orbit_end_transform = mOrbitEnd->getComponent<nap::TransformComponentInstance>();
		orbit_end_transform.setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mStartEnd[1]))));
		orbit_end_transform.setTranslate(glm::vec3(
			cos(nap::math::radians(-mStartEnd[1])) * mRadius * mUvScale,
			sin(nap::math::radians(-mStartEnd[1])) * mRadius * mUvScale,
			0.0f
		));

		// Update orbit sun position
		nap::TransformComponentInstance& orbit_sun_transform = mOrbitSun->getComponent<nap::TransformComponentInstance>();
		orbit_sun_transform.setTranslate(glm::vec3(
			cos(nap::math::radians(-getAngle())) * mRadius * mUvScale,
			sin(nap::math::radians(-getAngle())) * mRadius * mUvScale,
			0.0f
		));
	}

	void RandomOrbit::appendRenderableComponents(std::vector<nap::RenderableComponentInstance*>& renderable_components)
	{
		renderable_components.emplace_back(&mOrbitPath->getComponent<nap::RenderableMeshComponentInstance>());
		renderable_components.emplace_back(&mOrbitStart->getComponent<nap::RenderableMeshComponentInstance>());
		renderable_components.emplace_back(&mOrbitEnd->getComponent<nap::RenderableMeshComponentInstance>());
		renderable_components.emplace_back(&mOrbitSun->getComponent<nap::RenderableMeshComponentInstance>());
	}

	float RandomOrbit::getAngle() {
		return mStartEnd[0] - mProgress * (mStartEnd[0] - mStartEnd[1]);
	}
}
