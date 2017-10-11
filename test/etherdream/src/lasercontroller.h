#pragma once

// Local Includes
#include "lasercompound.h"

// External Includes
#include <nap/component.h>
#include <unordered_map>
#include <nap/entity.h>

namespace nap
{
	class LaserControlInstanceComponent;

	/**
	 *	lasercontroller
	 */
	class LaserControlComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LaserControlComponent, LaserControlInstanceComponent)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: holds all the compounds, creates a laser instance for every compound
		std::vector<ObjectPtr<nap::LaserCompound>> mLaserCompounds;

		// property: link to the entity that we want to create based on the compound
		nap::ObjectPtr<Entity> mLaserPrototype;
	};


	/**
	 * lasercontrollerInstance	
	 */
	class LaserControlInstanceComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LaserControlInstanceComponent(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize lasercontrollerInstance based on the lasercontroller resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the lasercontrollerInstance is initialized successfully
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 *	@return the laser entity associated with @id, nullptr if not found
		 */
		nap::EntityInstance* getLaserEntity(int id);

	private:
		// All the laser compounds associated with this controller
		std::vector<ObjectPtr<nap::LaserCompound>> mLaserCompounds;

		// All the instantiated laser prototypes
		std::unordered_map<int, nap::EntityInstance*> mLaserEntityMap;
	};
}
