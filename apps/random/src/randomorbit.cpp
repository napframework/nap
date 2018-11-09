#include "randomorbit.h"
#include "randomapp.h"

#include <mathutils.h>

namespace nap
{
	RandomOrbit::RandomOrbit(RandomApp& app) : mApp(app)
	{
		// Store scene objects
		mOrbit = mApp.mScene->findEntity("Orbit");
		mOrbitPath = mApp.mScene->findEntity("OrbitPath");
		mOrbitStart = mApp.mScene->findEntity("OrbitStart");
		mOrbitEnd = mApp.mScene->findEntity("OrbitEnd");
		mOrbitSun = mApp.mScene->findEntity("OrbitSun");
		
		// Call an intial update to apply properties
		updateOrbit();
	}

	void RandomOrbit::update(double deltaTime)
	{

	}

	void RandomOrbit::updateOrbit()
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

	float RandomOrbit::getAngle()
	{
		return mStartEnd[0] - mProgress * (mStartEnd[0] - mStartEnd[1]);
	}

	float RandomOrbit::getProgressByTime()
	{
		utility::DateTime currentDateTime = utility::getCurrentDateTime();
		int currentYear = currentDateTime.getYear();
		int currentMonth = static_cast<int>(currentDateTime.getMonth()) + 1;
		int currentDay = currentDateTime.getDayInTheMonth();
		utility::SystemTimeStamp currentTime = currentDateTime.getTimeStamp();
		utility::SystemTimeStamp startTime = utility::createTimestamp(currentYear, currentMonth, currentDay, mStartHour, 0);
		utility::SystemTimeStamp endTime = utility::createTimestamp(currentYear, currentMonth, currentDay, mEndHour, 0);
		if (currentTime <= startTime)
			return 0.0f;
		if (currentTime >= endTime)
			return 1.0f;
		utility::Seconds progress = std::chrono::duration_cast<utility::Seconds>(currentTime - startTime);
		utility::Seconds range = std::chrono::duration_cast<utility::Seconds>(endTime - startTime);
		return static_cast<float>(progress.count()) / static_cast<float>(range.count());
	}
}
