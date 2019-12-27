#pragma once

// Local Includes
#include "flexadapter.h"
#include "maccontroller.h"

// External Includes
#include <nap/resource.h>
#include <mutex>
#include <smoothdamp.h>

namespace nap
{
	/**
	 * Translates FLEX algorithm output together with raw overrides into motor output for the mac controller.
	 */
	class NAPAPI MACAdapter : public FlexAdapter
	{
		RTTI_ENABLE(FlexAdapter)
	public:
		virtual ~MACAdapter();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns the actual motor position values given to the mac-controller. Thread safe.
		 * These values are always up to date.
		 * @param outSteps current motor steps, thread safe.
		 */
		void getMotorInput(std::vector<float>& outSteps);

		/**
		 * Returns the motor lag. The lag = 0 when smoothing is turned off.
		 * To get the actual motor position values given to the mac-controller use getMotorSteps().
		 * The lag tells you how close a motor is to it's target value when smoothing is turned on.
		 * @param outSteps the smoother target values
		 * @param outVel the current smoothing velocity
		 */
		void getLag(std::vector<float>& outLag, std::vector<float>& outVel);

		/**
		 * Enables or disables the smoothing of motor positions. Thread safe.
		 * @param value if smoothing should be turned on or off
		 */
		void enableSmoothing(bool value);

		/**
		 * @return if smoothing of motor positions is enabled
		 */
		bool smoothingEnabled() const;

		/**
		 * Sets the motor smooth time in seconds.
		 * @param smoothTime time it will take to actual to target position
		 */
		void setSmoothTime(float smoothTime);

		/**
		 * @return motor smooth time in seconds
		 */
		float getSmoothTime();

		nap::ResourcePtr<MACController> mController = nullptr;		///< Property: 'Controller' the MAC controller that manages all the motor

		float	mMotorStepsPerMeter = 129473.41f;					///< Property: 'Motor Steps Per Meter' number of motor steps associated with a single meter
		int		mMotorStepOffset = 0;								///< Property: 'Motor Step Offset'
		
		float	mSmoothTime		= 2.0f;								///< Property: 'Smooth Time' time it will take to reach target value in seconds
		float	mMaxSmoothTime	= 1000000.0f;						///< Property: 'Max Smooth Speed' maximum smooth interpolation speed
		bool	mSmoothUsingVel = true;								///< Property: 'Smooth Using Velocity' if smooth time is scaled by velocity
		float	mSmoothMinVel	= 1000.0f;							///< Property: 'Min Smooth Velocity' velocity low cutoff point, smooth time will be 'Min Smooth Time'
		float	mSmoothMaxVel	= 25000.0f;							///< Property: 'Max Smooth Velocity' velocity max cutoff point, smooth time will be 'Smooth Time'
		float	mSmoothTimeMin	= 0.5f;								///< Property: 'Smooth Time Min' smooth time used when velocity is low.

		std::vector<int> mMotorMapping;								///< Property: 'Motor Mapping' flex to individual motor mapping

	protected:
		virtual void onCompute(const FlexDevice& device, double deltaTime) override;

		virtual void onStart() override;

		virtual void onEnable(bool value) override;

	private:

		using FlexSmoothPtr = std::unique_ptr<math::FloatSmoothOperator>;

		// Member variables
		std::vector<float> mMotorInput			= std::vector<float>(8);
		std::vector<float> mMotorStepsInt		= std::vector<float>(8);
		std::vector<FlexSmoothPtr> mSmoothers	= std::vector<FlexSmoothPtr>(8);
		std::vector<MacPosition> mMotorData		= std::vector<MacPosition>(8);
		std::atomic<bool> mEnableSmoothing		= { false };
		std::atomic<float> mSmoothTimeLocal		= { 1.0f };
		bool mSetStep							= false;
		bool mRestart							= false;
		float mVelocity							= 0.0f;
		std::mutex mMotorMutex;

		// Computes the lag
		void applyLag(std::vector<float>& outSteps, double deltaTime);
	};
}
