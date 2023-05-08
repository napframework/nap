/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <glm/glm.hpp>

namespace nap
{
	class FocusControllerInstance;

	/**
	 * Resource part of the focus controller.
	 * This is a perspective camera controller that rotates the camera around a target point - for quilt cameras.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FocusController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FocusController, FocusControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<TransformComponent> mLookAtTargetComponent;	///< Property: 'mLookAtTarget' transform of the world space position to look at	 .
	};


	/**
	 * Instance part of the orbit controller.
	 * The controller is a perspective camera controller that rotates the camera around a target point.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target, middle button to
	 * move the camera along its current direction vector to the optimal focus distance from the target transform.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FocusControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FocusControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Update this component
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the focal position the camera. 
		 */
		glm::vec3 getFocalPosition() const;

		/**
		 * @return the current lookat target transform
		 */
		const TransformComponentInstance& getLookAtTarget() const		{ return *mLookAtTargetComponent; }

	private:
		FocusController*							mResource = nullptr;

		ComponentInstancePtr<TransformComponent>	mLookAtTargetComponent = { this, &FocusController::mLookAtTargetComponent };
		TransformComponentInstance*					mTransformComponent = nullptr;		// The transform component used to move the entity
	};

}
