/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include <mathutils.h>

// External Includes
#include <rtti/rtti.h>

namespace nap
{
	namespace math
	{
		/**
		 * Base class of operator that gradually changes a value towards a desired goal over time
		 */
		class BaseSmoothOperator
		{
			RTTI_ENABLE()
		public:

			/**
			 *	Default constructor
			 */
			BaseSmoothOperator() = default;

			/**
			* Constructor that takes a smooth time
			* @param smoothTime the time in seconds it will take to reach the target
			*/
			BaseSmoothOperator(float smoothTime) : mSmoothTime(smoothTime)	{ }

			/**
			 * Constructor that takes a smooth time and max speed
			 * @param smoothTime the time in seconds it will take to reach the target
			 * @param maxSpeed the maximum blend speed
			 */
			BaseSmoothOperator(float smoothTime, float maxSpeed) :
				mSmoothTime(smoothTime),
				mMaxSpeed(maxSpeed)											{ }

			// Default destructor
			virtual ~BaseSmoothOperator() = default;

			float mSmoothTime = 1.0f;						// approximately the time it will take to reach the target. A smaller value will reach the target faster.
			float mMaxSpeed = math::max<float>();			// allows you to clamp the maximum blend speed 
		};


		/**
		 * Smooth Damp utility class. Gradually changes a value of type T towards a desired goal over time.
		 * This operator needs to be updated every frame! 
		 * Call update() together with the new deltaTime and desired target value on application update. 
		 * The update call returns the new smoothed value. Alternatively use getValue()
		 * to get the current smoothed value after update. 
		 * This operator is a convenience wrapper around nap::math::smoothDamp<T>()
		 */
		template<typename T>
		class SmoothOperator : public BaseSmoothOperator
		{
			RTTI_ENABLE(BaseSmoothOperator)
		public:
			/**
			* Constructor that creates the operator based on a set of initial values
			* @param currentValue the value it should started blending from, often same as initial target
			* @param smoothTime the time in seconds it will take to reach the target
			*/
			SmoothOperator(const T& currentValue, float smoothTime);

			/**
			 * Constructor that creates the operator based on a set of initial values
			 * @param currentValue the value it should started blending from, often same as initial target
			 * @param smoothTime the time in seconds it will take to reach the target
			 * @param maxSpeed the maximum blend speed
			 */
			SmoothOperator(const T& currentValue, float smoothTime, float maxSpeed);

			/**
			 * Updates the current blend value based on the given targetValue
			 * @param targetValue the value to blend to
			 * @param deltaTime time in seconds it took to complete the last compute cycle
			 * @return the current blend value
			 */
			T& update(const T& targetValue, float deltaTime);

			/**
			 * @return the current blend value
			 */
			const T& getValue() const								{ return mValue; }

			/**
			 * @return the current blend value
			 */
			T& getValue()											{ return mValue; }

			/**
			 * @return the current target value
			 */
			const T& getTarget() const								{ return mTarget; }

			/**
			 * @return the current target value
			 */
			T& getTarget()											{ return mTarget; }

			/**
			 * @return current velocity
			 */
			const T& getVelocity() const							{ return mVelocity; }

			/**
			 * @return the current velocity
			 */
			T& getVelocity()										{ return mVelocity; }

			/**
			 * Sets the current blend value.
			 * @param value the value to set as blend value
			 */
			void setValue(const T& value);

		protected:
			/**
			 * This needs to be implemented by every template specialized class
			 * Should set 
			 */
			void init();

			T mValue;			// Current blend value
			T mTarget;			// Current blend target
			T mVelocity;		// Current velocity
		};


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		T& nap::math::SmoothOperator<T>::update(const T& targetValue, float deltaTime)
		{
			mTarget = targetValue;
			nap::math::smooth<T>(mValue, targetValue, mVelocity, deltaTime, mSmoothTime, mMaxSpeed);
			return mValue;
		}

		template<typename T>
		nap::math::SmoothOperator<T>::SmoothOperator(const T& currentValue, float smoothTime, float maxSpeed) : BaseSmoothOperator(smoothTime, maxSpeed),
			mValue(currentValue)
		{
			init();
		}

		template<typename T>
		nap::math::SmoothOperator<T>::SmoothOperator(const T& currentValue, float smoothTime) : BaseSmoothOperator(smoothTime), mValue(currentValue)
		{
			init();
		}


		//////////////////////////////////////////////////////////////////////////
		// Type definitions for all supported smooth operators
		//////////////////////////////////////////////////////////////////////////
		using FloatSmoothOperator = SmoothOperator<float>;
		using Vec2SmoothOperator  = SmoothOperator<glm::vec2>;
		using Vec3SmoothOperator  = SmoothOperator<glm::vec3>;
		using Vec4SmoothOperator  = SmoothOperator<glm::vec4>;


		//////////////////////////////////////////////////////////////////////////
		// Forward declarations
		//////////////////////////////////////////////////////////////////////////
		template<>
		NAPAPI void nap::math::SmoothOperator<float>::init();

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec2>::init();

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec3>::init();

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec4>::init();

		template<>
		NAPAPI void nap::math::SmoothOperator<float>::setValue(const float& value);

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec2>::setValue(const glm::vec2& value);

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec3>::setValue(const glm::vec3& value);

		template<>
		NAPAPI void nap::math::SmoothOperator<glm::vec4>::setValue(const glm::vec4& value);
	}
}