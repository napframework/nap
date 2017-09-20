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
		DECLARE_COMPONENT(FrustrumSyncComponent, FrustrumSyncComponentInstance)

	public:
		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	class FrustrumSyncComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		FrustrumSyncComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)	{ }

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update the size of the transform based on the laser frustrum
		virtual void update(double deltaTime) override;

	private:
		// Object pointer to laser
		LaserOutputComponentInstance* mOutput = nullptr;

		// Object pointer to transform
		TransformComponentInstance* mTransform;
	};

}