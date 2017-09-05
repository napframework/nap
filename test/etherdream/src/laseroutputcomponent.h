#pragma once

// Local Includes
#include "etherdreamdac.h"
#include "lasershapecomponent.h"

// External Includes
#include <nap/component.h>

namespace nap
{
	class LaserOutputComponentInstance;

	/**
	 *	Holds a reference to a laser dac and selects which shape to send data to
	 */
	class LaserOutputComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(LaserOutputComponentInstance);
		}

		// Index property of component to select
		int mIndex = 0;

		// Link to the DAC
		ObjectPtr<EtherDreamDac> mDac;
	};


	/**
	 *	Selects the shape and uploads points
	 */
	class LaserOutputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		LaserOutputComponentInstance(EntityInstance& entity, Component& resource) : 
			ComponentInstance(entity, resource)
		{
		}

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;

		// Pointer to the dac
		ObjectPtr<EtherDreamDac> mDac;

		// Index to send
		int mIndex = 0;

	private:
		// All the available shapes to draw
		std::vector<LaserShapeComponentInstance*> mShapes;

	};
}