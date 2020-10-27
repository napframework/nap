/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * Syncs the size of the canvas on screen to the dimensions of the current laser bounds
	 * This ensures that the canvas represents the right laser display bounds
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

		// Object pointer to transform of laser canvas
		TransformComponentInstance* mCanvasTransform = nullptr;
	};

}