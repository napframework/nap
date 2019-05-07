#include "orbitcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/datetime.h>

// nap::OrbitComponent run time class definition 
RTTI_BEGIN_CLASS(nap::OrbitComponent)
	RTTI_PROPERTY("OrbitTransformComponent", &nap::OrbitComponent::mOrbitTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitPathTransformComponent", &nap::OrbitComponent::mOrbitPathTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitStartTransformComponent", &nap::OrbitComponent::mOrbitStartTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitEndTransformComponent", &nap::OrbitComponent::mOrbitEndTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitSunTransformComponent", &nap::OrbitComponent::mOrbitSunTransformComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::OrbitComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrbitComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void OrbitComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool OrbitComponentInstance::init(utility::ErrorState& errorState)
	{
		// Call an intial update to apply properties
		updateOrbit();

		return true;
	}


	void OrbitComponentInstance::update(double deltaTime)
	{

	}


	void OrbitComponentInstance::updateOrbit()
	{
		// Update orbit position
		mOrbitTransformComponent->setTranslate(glm::vec3(
			mUvOffset.x + mCenter[0] * mUvScale,
			mOrbitTransformComponent->getTranslate().y,
			mUvOffset.y + mCenter[1] * -mUvScale
		));

		// Update orbit path scale
		mOrbitPathTransformComponent->setUniformScale(mRadius * mUvScale);

		// Update orbit start position and rotation
		mOrbitStartTransformComponent->setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mStartEnd[0]))));
		mOrbitStartTransformComponent->setTranslate(glm::vec3(
			cos(nap::math::radians(-mStartEnd[0])) * mRadius * mUvScale,
			sin(nap::math::radians(-mStartEnd[0])) * mRadius * mUvScale,
			0.0f
		));

		// Update orbit end position and rotation
		mOrbitEndTransformComponent->setRotate(glm::quat(glm::vec3(0.0f, 0.0f, nap::math::radians(-mStartEnd[1]))));
		mOrbitEndTransformComponent->setTranslate(glm::vec3(
			cos(nap::math::radians(-mStartEnd[1])) * mRadius * mUvScale,
			sin(nap::math::radians(-mStartEnd[1])) * mRadius * mUvScale,
			0.0f
		));

		// Update orbit sun position
		mOrbitSunTransformComponent->setTranslate(glm::vec3(
			cos(nap::math::radians(-getAngle())) * mRadius * mUvScale,
			sin(nap::math::radians(-getAngle())) * mRadius * mUvScale,
			0.0f
		));
	}


	float OrbitComponentInstance::getAngle()
	{
		return mStartEnd[0] - mProgress * (mStartEnd[0] - mStartEnd[1]);
	}


	float OrbitComponentInstance::getProgressByTime()
	{
		DateTime currentDateTime = getCurrentDateTime();
		int currentYear = currentDateTime.getYear();
		int currentMonth = static_cast<int>(currentDateTime.getMonth()) + 1;
		int currentDay = currentDateTime.getDayInTheMonth();
		SystemTimeStamp currentTime = currentDateTime.getTimeStamp();
		SystemTimeStamp startTime = createTimestamp(currentYear, currentMonth, currentDay, mStartHour, 0);
		SystemTimeStamp endTime = createTimestamp(currentYear, currentMonth, currentDay, mEndHour, 0);
		if (currentTime <= startTime)
			return 0.0f;
		if (currentTime >= endTime)
			return 1.0f;
		Seconds progress = std::chrono::duration_cast<Seconds>(currentTime - startTime);
		Seconds range = std::chrono::duration_cast<Seconds>(endTime - startTime);
		return static_cast<float>(progress.count()) / static_cast<float>(range.count());
	}
}
