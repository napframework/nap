#pragma once

// Local Includes
#include "laseroutputcomponent.h"
#include "polyline.h"

// External Includes
#include <component.h>
#include <transformcomponent.h>
#include <entity.h>

namespace nap
{
	class FrustumSyncComponentInstance;

	/**
	* This component syncs the size of the canvas to the frustrum of the laser
	*/
	class NAPAPI FrustumSyncComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FrustumSyncComponent, FrustumSyncComponentInstance)

	public:
		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Property: The output component this component uses to resolve the canvas size
		nap::ComponentPtr<LaserOutputComponent> mLaserOutputComponent;
	};


	/**
	 *	When created this object spawns a laser canvas entity that is synced every update to the canvas of the laser
	 */
	class NAPAPI FrustumSyncComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		FrustumSyncComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)	{ }

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		// Update the size of the transform based on the laser frustrum
		virtual void update(double deltaTime) override;

	private:
		// Object pointer to laser
		ComponentInstancePtr<LaserOutputComponent> mOutput = { this, &FrustumSyncComponent::mLaserOutputComponent };

		// Object pointer to transform of visualizer
		TransformComponentInstance* mCanvasTransform;
	};

}