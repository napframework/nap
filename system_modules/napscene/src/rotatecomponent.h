/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "component.h"

// External includes
#include <utility/dllexport.h>

// Local includes
#include "transformcomponent.h"

#include <glm/glm.hpp>

namespace nap
{
	/**
	 * User configurable properties for the rotate component
	 */
	struct NAPAPI RotateProperties
	{
		glm::vec3	mAxis	= {0.0f, 1.0f, 0.0f};	///< Property: 'Axis' Rotation axis
		float		mSpeed	= 1.0f;					///< Property: 'Speed' Rotation speed (seconds)
		float		mOffset	= 0.0f;					///< Property: 'Offset' Rotation offset in seconds
	};

	//////////////////////////////////////////////////////////////////////////

	class RotateComponentInstance;

	/**
	 * Resource part of the rotate component.
	 * Automatically rotates the entity along a certain axis at a certain speed.
	 * The initial rotation value after initialization is used to rotate along the specified axis.
	 * This component updates (overrides) the rotate value of the transform component.
	 * Entities that use this component must have a transform.
	 */
	class NAPAPI RotateComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RotateComponent, RotateComponentInstance)
	public:
		/**
		* Uses transform to rotate itself in the world.
		*/
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		RotateProperties mProperties;
		bool mEnabled = true;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance part of the rotate component. 
	 * Automatically rotates the entity along a certain axis at a certain speed.
	 * The initial rotation value after initialization is used to rotate along the specified axis.
	 * This component updates (overrides) the rotate value of the transform component.
	 * Entities that use this component must have a transform.
	 */
	class NAPAPI RotateComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RotateComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)		{ }

		/**
		 * Initialize this rotate component, copies it's members over and validates
		 * if a transform component is available that can be rotated.
		 * @param errorState contains the error if initialization fails.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Rotates the component based on the current speed and axis
		 * @param deltaTime frame time in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Resets rotation to be 0
		 */
		void reset();

		/**
		 * Enable or disable the rotation
		 */
		void enable(bool enable)									{ mEnabled = enable; }

		/**
		 * @return whether the component is enabled
		 */
		bool isEnabled() const										{ return mEnabled; }

		/*
		 * Sets the rotation speed
		 * @param speed rotation speed in seconds
		 */
		void setSpeed(float speed)									{ mProperties.mSpeed = speed; }

		/**
		 * @return the rotation speed in seconds
		 */
		float getSpeed() const										{ return mProperties.mSpeed; }

		/**
		 * Sets the rotation axis
		 * @param axis rotation axis
		 */
		void setAxis(const glm::vec3& axis)							{ mProperties.mAxis = axis; }

		/**
		 * @return the rotation axis
		 */
		glm::vec3 getAxis() const									{ return mProperties.mAxis; }

		// Rotation properties
		RotateProperties mProperties;

	private:
		// Store pointer to transform, set during init
		nap::TransformComponentInstance* mTransform = nullptr;

		// Local elapsed time
		double mElapsedTime = 0.0;

		// Enable flag
		bool mEnabled = true;

		// Initial Rotation value
		glm::quat mInitialRotate = glm::quat();
	};
}
