#pragma once

// Local Includes
#include "laseroutputcomponent.h"

// External Includes
#include <nap/component.h>
#include "polyline.h"
#include <transformcomponent.h>

namespace nap
{
	class FrustrumSyncComponentInstance;

	/**
	* This component syncs the size of the canvas to the frustrum of the laser
	*/
	class FrustrumSyncComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(FrustrumSyncComponentInstance);
		}

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) override;
	};


	class FrustrumSyncComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		FrustrumSyncComponentInstance(EntityInstance& entity) : ComponentInstance(entity)	{ }

		// Init
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update the size of the transform based on the laser frustrum
		virtual void update(double deltaTime) override;

	private:
		// Object pointer to laser
		LaserOutputComponentInstance* mOutput = nullptr;

		// Object pointer to transform
		TransformComponentInstance* mTransform;
	};

}