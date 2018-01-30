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
		 *	Base class of operator that gradually changes a value towards a desired goal over time
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
			float mMaxSpeed = 10000.0f;						// allows you to clamp the maximum blend speed 
		};


		/**
		 * Smooth Damp utility class
		 * This object gradually changes value T towards a desired goal over time
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
			 * @return the current blend value
			 */
			const T& getValue() const								{ return mValue; }

			/**
			 * @return the current blend value
			 */
			T& getValue()											{ return mValue; }

			/**
			 * @return current velocity
			 */
			const T& getVelocity() const							{ return mVelocity; }

			/**
			 * @return the current velocity
			 */
			T& getVelocity()										{ return mVelocity; }

			/**
			 * Updates the current blend value based on @targetValue
			 * @param targetValue the value to blend to
			 * @param deltaTime time in seconds it took to complete the last compute cycle
			 * @return the current blend value
			 */
			T& update(const T& targetValue, float deltaTime);

			/**
			 * Sets the current blend value
			 * @param value the value to set as blend value
			 */
			void setValue(const T& value)							{ mValue = value; }

		protected:
			/**
			 * This needs to be implemented by every template specialized class
			 * Should set 
			 */
			void init();

			T mValue;			// Current blend value
			T mVelocity;		// Current velocity
		};


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		T& nap::math::SmoothOperator<T>::update(const T& targetValue, float deltaTime)
		{
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
	}
}