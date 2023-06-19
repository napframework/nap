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
		 * Rotates the component every tick based on the speed and exis
		 * @param deltaTime time it took to complete last cook (seconds)
		 */
		virtual void update(double deltaTime) override;

		/**
		* Resets rotation to be 0
		*/
		void reset();

		// Rotation properties
		RotateProperties mProperties;

	private:
		// Store pointer to transform, set during init
		nap::TransformComponentInstance* mTransform;

		// Local elapsed time
		double mElapsedTime = 0.0;

		// Initial Rotation value
		glm::quat mInitialRotate = glm::quat();
	};
}
