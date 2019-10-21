#pragma once

// Local Includes
#include "maccontroller.h"
#include "flexblockdata.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
	class FlexAdapter;

	/**
	 * The flexblock algorithm.
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

		ResourcePtr<FlexBlockShape>				mFlexBlockShape;				///< Property: 'FlexBlockShape' Reference to the shape definition of the block.
		std::vector<ResourcePtr<FlexAdapter>>	mAdapters;						///< Property: 'Adapters' All flexblock adapters.

		// Properties
		int					mUpdateFrequency = 1000;							///< Property: 'Frequency' device operation frequency in hz.
		float				mOverrideMinimum = 0.0f;							///< Property: 'Override Minimum' minimum of override parameters in meters, we start to count from this value
		float				mSlack = 0.0f;										///< Property: 'Slack Range' 
		float				mSinusAmplitude = 0.0f;								///< Property: 'Sinus Amplitude'			
		float				mSinusFrequency = 0.0f;								///< Property: 'Sinus Frequency
	};
}