#include "laseroutputcomponent.h"
#include "makeprototypecomponent.h"
#include "osclaserinputhandler.h"

// External Includes
#include <nap/entity.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>

// nap::makeprototypecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::MakePrototypeComponent)
	RTTI_PROPERTY("SplineEntity", &nap::MakePrototypeComponent::mSplineEntity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputEntity", &nap::MakePrototypeComponent::mLaserOutputEntity, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::makeprototypecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MakePrototypeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void MakePrototypeComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool MakePrototypeComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get resource and manager
		MakePrototypeComponent* resource = getComponent<MakePrototypeComponent>();
		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();

		// Create the entities
		mSplineEntity = resource_manager.createEntity(*(resource->mSplineEntity), entityCreationParams, errorState).get();
		if (mSplineEntity == nullptr)
			return false;
		getEntityInstance()->addChild(*mSplineEntity);

		mLaserOutputEntity = resource_manager.createEntity(*(resource->mLaserOutputEntity), entityCreationParams, errorState).get();
		if (mLaserOutputEntity == nullptr)
			return false;
		getEntityInstance()->addChild(*mLaserOutputEntity);

		// Set-up relationships

		// The laser output component needs to know where the line is relative to it's canvas, therefore we provide it with the line entity
		nap::LaserOutputComponentInstance* laser_output_comp = mLaserOutputEntity->findComponent<nap::LaserOutputComponentInstance>();
		assert(laser_output_comp != nullptr);
		laser_output_comp->setTransform(*mSplineEntity);

		// The OSC Input handler needs a reference to the laser output component entity to figure out where it can place the line (bounds)
		nap::OSCLaserInputHandlerInstance* osc_handler = mSplineEntity->findComponent<OSCLaserInputHandlerInstance>();
		osc_handler->setLaserOutput(*mLaserOutputEntity);

		return true;
	}
}