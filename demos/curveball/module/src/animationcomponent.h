/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <fcurve.h>
#include <nap/resourceptr.h>
#include <transformcomponent.h>

namespace nap
{
	// Forward declares
	class TransformComponent;
	class AnimatorComponentInstance;
	
	/**
	 * Resource part of the Component.
	 * The instance sets the y value of a transform based on the value evaluated from a curve resource.
	 */
	class AnimatorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(AnimatorComponent, AnimatorComponentInstance)

		/**
		 * This component uses a transform
		 */
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		nap::ResourcePtr<nap::math::FloatFCurve> mCurve;		///< Property: 'Curve' The animation curve
	};


	/**
	* Instance part of the Component.
	* Updates the y value of a transform based on the value evaluated from a curve resource.
	*/
	class AnimatorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		AnimatorComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Called on initialization of this component
		 * Fetches the transform and curve resource needed by the update call
		 * @param errorState contains the error if initialization fails
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Called every frame by the app loop
		 * @param deltaTime time in between frames in seconds
		 */
		void update(double deltaTime) override;

		/**
		 * @return the current value of the sampled curve
		 */
		float getCurveValue() const							{ return mCurveValue; }

	private:
		float mLocalTime  = 0.0f;							///< Local Animation time
		float mCurveValue = 0.0f;							///< Current Curve Value
		TransformComponentInstance* mTransform = nullptr;	///< Transform component of the sphere
		math::FloatFCurve* mCurve = nullptr;				///< Pointer to the curve we want to sample

	};
}
