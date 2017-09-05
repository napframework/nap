#include "lasershapecomponent.h"
#include <nap/entity.h>

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::LaseShapeProperties)
	RTTI_PROPERTY("PointCount", &nap::LaseShapeProperties::mNumberOfPoints, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",		&nap::LaseShapeProperties::mSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",		&nap::LaseShapeProperties::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness",	&nap::LaseShapeProperties::mBrightness,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LaserShapeComponent)
	RTTI_PROPERTY("Properties", &nap::LaserShapeComponent::mShapeProperties, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserShapeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	bool LaserShapeComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy shape properties
		LaserShapeComponent* laser_shape_resource = getResource<LaserShapeComponent>();
		mShapeProperties = laser_shape_resource->mShapeProperties;

		// Create point buffer
		mPoints.clear();
		mPoints.resize(mShapeProperties.mNumberOfPoints);

		return true;
	}


	// Calculate current time for future laser shape effects
	void LaserShapeComponentInstance::update(double deltaTime)
	{
		mCurrentTime += (deltaTime * mShapeProperties.mSpeed);
	}

}