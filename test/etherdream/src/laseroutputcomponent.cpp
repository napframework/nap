#include "lasershapecomponent.h"
#include "laseroutputcomponent.h"

#include <nap/entity.h>
#include <nap/logger.h>
#include <mathutils.h>

using namespace nap::math;

RTTI_BEGIN_CLASS(nap::LaserOutputComponent)
	RTTI_PROPERTY("Index",	&nap::LaserOutputComponent::mIndex,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Dac",	&nap::LaserOutputComponent::mDac,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserOutputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	bool LaserOutputComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy over link to the DAC
		LaserOutputComponent* selector_resource = rtti_cast<LaserOutputComponent>(resource.get());
		mDac = selector_resource->mDac;

		// Get all the laser shapes
		this->getEntity()->getComponentsOfType<LaserShapeComponentInstance>(mShapes);

		// Clamp range of index
		mIndex = min<int>(selector_resource->mIndex, mShapes.size()-1);

		// Make sure we have some shapes to choose from
		return errorState.check(mShapes.size() > 0, "No laser shape components found to select from");
	}


	void LaserOutputComponentInstance::update(double deltaTime)
	{
		assert(mIndex < mShapes.size());
		LaserShapeComponentInstance* instance = mShapes[mIndex];

		// Don't do anything when the dac isn't connected
		mDac->setPoints(instance->getPoints());
	}
}