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
		std::vector<int> mMotorMapping;								///< Property: 'Motor Mapping' flex to individual motor mapping

	protected:
		virtual void onCompute(const FlexDevice& device) override;

	private:
		// Member variables
		std::vector<float> mRopeLengths		= std::vector<float>(8);
		std::vector<MacPosition> mMotorData = std::vector<MacPosition>(8);
	};
}
