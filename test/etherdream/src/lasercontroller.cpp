#include "lasercontroller.h"

// External Includes
#include <nap/entity.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>

// Local Includes
#include "makeprototypecomponent.h"

// nap::lasercontroller run time class definition 
RTTI_BEGIN_CLASS(nap::LaserControlComponent)
	RTTI_PROPERTY("LaserCompounds",		&nap::LaserControlComponent::mLaserCompounds, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PrototypeEntity",	&nap::LaserControlComponent::mLaserPrototype, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::lasercontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserControlInstanceComponent)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void LaserControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool LaserControlInstanceComponent::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get all compounds to use and create laser prototypes from
		nap::LaserControlComponent* resource = getComponent<LaserControlComponent>();
		mLaserCompounds = resource->mLaserCompounds;
		
		// Get resource manager that is used to spawn the new entity
		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();

		for (auto& compound : mLaserCompounds)
		{
			auto new_entity = resource_manager.createEntity(*(resource->mLaserPrototype), entityCreationParams, errorState);
			if (new_entity == nullptr)
				return false;

			// Find make prototype component instance
			MakePrototypeComponentInstance* prototype_component = new_entity->findComponent<MakePrototypeComponentInstance>();
			if (!errorState.check(prototype_component != nullptr, "laser entity doesn't have a make prototype component"))
				return false;

			// Make sure we don't have one with the same id
			if (mLaserEntityMap.find(compound->mLaserID) != mLaserEntityMap.end())
				return errorState.check(false, "laser with id %s already exists", compound->mLaserID);

			// TODO: Populate with laser compound settings
			if (!prototype_component->setup(*(compound), errorState))
				return false;

			// Store for future use
			mLaserEntityMap.emplace(std::make_pair(compound->mLaserID, new_entity.get()));

			// Add entity as child
			getEntityInstance()->addChild(*new_entity);
		}

		return true;
	}


	nap::EntityInstance* LaserControlInstanceComponent::getLaserEntity(int id)
	{
		auto it = mLaserEntityMap.find(id);
		if (it == mLaserEntityMap.end())
			return nullptr;
		return it->second;
	}

}