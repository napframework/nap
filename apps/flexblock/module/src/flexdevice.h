#pragma once

// Local Includes
#include "maccontroller.h"
#include "flexblockdata.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * flexdevice
	 */
	class NAPAPI FlexDevice : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~FlexDevice() override;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the device
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;

		nap::ResourcePtr<MACController>		mMacController = nullptr;		///< Property: 'MACController' Reference to the mac controller
		ResourcePtr<FlexBlockShape>			mFlexBlockShape;				///< Property: 'FlexBlockShape' Reference to the shape definition of the block 

		// Properties
		int					mFrequency = 1000;									///< Property: 'Frequency' Flexblock operation frequency
		float				mOverrideMinimum = 0.0f;							///< Property: 'Override Minimum' minimum of override parameters in meters, we start to count from this value
		float				mOverrideRange = 24.0f;								///< Property: 'Override Range' range of override parameters in meters
		float				mSlackRange = 1.0f;									///< Property: 'Slack Range' 
		float				mSinusAmplitude = 0.0f;								///< Property: 'Sinus Amplitude'			
		float				mSinusFrequency = 0.0f;								///< Property: 'Sinus Frequency
		double				mMotorStepsPerMeter = 129473.41;					///< Property: 'Motor Steps Per Meter' Number of motor steps associated with a meter
		int					mMotorStepOffset = 0;								///< Property: 'Motor Step Offset'
		bool				mEnableController = false;							///< Property: 'Enable Controller' If the mac controller is enabled
		std::vector<int>	mMotorMapping = { 5, 1, 2, 6, 3, 7, 0, 4 };			///< Property: 'Motor Mapping' How the algorithm outputs map to the individual motors
	};
}
