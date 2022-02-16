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
	class BoidTargetTranslateComponentInstance;

	/**
	 * Resource part of the target translate component.
	 * Automatically rotates the entity along a certain axis at a certain speed.
	 * This component updates (overrides) the translation value of the transform component.
	 * Entities that use this component must have a transform.
	 */
	class NAPAPI BoidTargetTranslateComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(BoidTargetTranslateComponent, BoidTargetTranslateComponentInstance)
	public:
		/**
		* Uses transform to rotate itself in the world.
		*/
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		float mRadius = 1.0f;					///< Property: 'Radius' Radius
		float mSpeed = 1.0f;					///< Property: 'Speed' Rotation speed
	};


	/**
	 * Instance part of the target translate component. 
	 * Automatically translates the entity randomly within a specified radius.
	 * The initial rotation value after initialization is used to rotate along the specified axis.
	 * This component updates (overrides) the rotate value of the transform component.
	 * Entities that use this component must have a transform.
	 */
	class NAPAPI BoidTargetTranslateComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		BoidTargetTranslateComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)		{ }

		/**
		 * Initialize this target translate component, copies it's members over and validates
		 * if a transform component is available that can be translated.
		 * @param errorState contains the error if initialization fails.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Translates the component every tick based on the speed
		 * @param deltaTime time it took to complete last cook (seconds)
		 */
		virtual void update(double deltaTime) override;

		/**
		* Resets translation to origin
		*/
		void reset();

		float mRadius;
		float mSpeed;

	private:
		// Store pointer to transform, set during init
		nap::TransformComponentInstance* mTransform;

		// Forward direction
		const glm::vec3 mForward = { 0.0f, 0.0f, 1.0f };

		// Local elapsed time
		double mElapsedTime = 0.0;
	};
}
