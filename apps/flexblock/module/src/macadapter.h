#pragma once

// Local Includes
#include "flexadapter.h"
#include "maccontroller.h"

// External Includes
#include <nap/resource.h>
#include <mutex>

namespace nap
{
	/**
	 * Translates FLEX algorithm calls into motor output for the mac controller
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

		nap::ResourcePtr<MACController> mController = nullptr;		///< Property: 'Controller' the MAC controller that manages all the motor

		bool	mSetDigitalPin	= false;							///< Property: 'Set Digital Pin' if the digital pin is controlled by this adapter
		float	mMotorStepsPerMeter = 129473.41f;					///< Property: 'Motor Steps Per Meter' number of motor steps associated with a single meter
		int		mMotorStepOffset = 0;								///< Property: 'Motor Step Offset'
		float	mSinusAmplitude = 0.0f;								///< Property: 'Sinus Amplitude'			
		float	mSinusFrequency = 0.0f;								///< Property: 'Sinus Frequency
		float	mOverrideMinimum = 0.0f;							///< Property: 'Override Minimum' minimum of override parameters in meters, we start to count from this value
		std::vector<int> mMotorMapping;								///< Property: 'Motor Mapping' flex to individual motor mapping

		/**
		 * Applies individual motor override settings (0-8). This call is thread safe.
		 * @param inputs new motor override values
		 */
		void setMotorOverrides(const std::vector<float>& inputs);

		/**
		 * Returns the individual motor overrides (0-8). This call is thread safe.
		 * @param outOverrides current override values
		 */
		void getMotorOverrides(std::vector<float>& outOverrides) const;

	protected:
		virtual void onCompute(const FlexDevice& device) override;

	private:
		mutable std::mutex mOverrideMutex;						///< Mutex used to get / set motor override values

		// Member variables
		std::vector<float> mRopeLengths		= std::vector<float>(8);
		std::vector<float> mMotorOverrides	= std::vector<float>(8);
	};
}
