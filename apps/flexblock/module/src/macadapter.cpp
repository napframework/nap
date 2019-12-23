#include "macadapter.h"
#include "flexdevice.h"

#include <nap/logger.h>

// nap::macadapter run time class definition 
RTTI_BEGIN_CLASS(nap::MACAdapter)
	RTTI_PROPERTY("Motor Steps Per Meter",	&nap::MACAdapter::mMotorStepsPerMeter,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Step Offset",		&nap::MACAdapter::mMotorStepOffset,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Smooth Time",			&nap::MACAdapter::mSmoothTime,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Max Smooth Speed",		&nap::MACAdapter::mMaxSmoothTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Mapping",			&nap::MACAdapter::mMotorMapping,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controller",				&nap::MACAdapter::mController,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	MACAdapter::~MACAdapter()			{ }


	bool MACAdapter::init(utility::ErrorState& errorState)
	{
		// Ensure we have enough mappings for the slaves
		if (!errorState.check(mController->getSlaveCount() < mMotorMapping.size(), "slave count exceeds number of motor mappings"))
			return false;

		// Create our smoothers
		setSmoothTime(mSmoothTime);
		for (auto& smoother : mSmoothers)
		{
			smoother = std::make_unique<math::FloatSmoothOperator>(0, mSmoothTimeLocal, mMaxSmoothTime);
		}
		return true;
	}


	void MACAdapter::onCompute(const FlexDevice& device, double deltaTime)
	{
		//////////////////////////////////////////////////////////////////////////
		// Convert flexblock output into motor values
		//////////////////////////////////////////////////////////////////////////

		// Extract rope lengths
		device.getRopeLengths(mMotorStepsInt);

		// convert meters to motor-steps
		for (auto& length : mMotorStepsInt)
			length *= mMotorStepsPerMeter;

		// Smoothly interpolate values if enabled.
		// Always copy at the end of the cycle
		{
			std::lock_guard<std::mutex> lock(mMotorMutex);
			if (mEnableSmoothing)
			{
				// Lock for smoothing
				int idx = 0;
				float smooth_time = mSmoothTimeLocal;
				for (auto& smoother : mSmoothers)
				{
					smoother->mSmoothTime = smooth_time;
					mMotorStepsInt[idx] = smoother->update(mMotorStepsInt[idx], deltaTime);
					idx++;
				}
			}

			// Store steps
			mMotorInput = mMotorStepsInt;
		}

		//////////////////////////////////////////////////////////////////////////
		// Set motor values in controller
		//////////////////////////////////////////////////////////////////////////

		// Don't perform any tasks when the controller isn't running
		if (!mController->isRunning())
			return;

		// check for all slaves to be operational and without errors
		// if any slave isn't operational stop processing
		for (int i = 0; i < mController->getSlaveCount(); i++)
		{
			if (mController->getSlaveState(i) != EtherCATMaster::ESlaveState::Operational)
			{
				nap::Logger::error("Slave: %d not operational! Stopping MACController!", i);
				mController->stop();
				return;
			}
		}

		// Ensure out input data matches number of slaves
		// This also ensures the initial set of motor positions matches that of the controller
		mController->getPositionData(mMotorData);

		// All slaves are operational
		for (int i = 0; i < mController->getSlaveCount(); i++)
		{
			// get the mapped motor index
			int mapped_idx = mMotorMapping[i];

			// Check if the mapped index doesn't exceed number of slaves
			if (mapped_idx >= mController->getSlaveCount())
				continue;

			// Update target position
			mMotorData[i].setTargetPosition(mMotorStepsInt[mapped_idx]);
		}

		// Update position data
		mController->setPositionData(mMotorData);
	}


	void MACAdapter::getMotorInput(std::vector<float>& outSteps)
	{
		std::lock_guard<std::mutex> lock(mMotorMutex);
		outSteps = mMotorInput;
	}


	void MACAdapter::getLag(std::vector<float>& outLag, std::vector<float>& outVel)
	{
		// No lag when smoothing is disabled
		if (!mEnableSmoothing)
		{
			const static std::vector<float> empty(8, 0);
			outLag = empty;
			outVel = empty;
			return;
		}

		// Setup 
		outLag.clear();
		outLag.reserve(mSmoothers.size());

		outVel.clear();
		outVel.reserve(mSmoothers.size());
		
		std::lock_guard<std::mutex> lock(mMotorMutex);
		assert(mMotorInput.size() == mSmoothers.size());

		for (auto i = 0; i < mMotorInput.size(); i++)
		{
			outLag.emplace_back(math::abs<float>(mSmoothers[i]->getTarget() - mMotorInput[i]));
			outVel.emplace_back(mSmoothers[i]->getVelocity());
		}
	}


	void MACAdapter::enableSmoothing(bool value)
	{
		// Turn off
		if (!value)
		{
			mEnableSmoothing = false;
			return;
		}
		
		// Now update smoothers, the reference position is updated. 
		// This ensures that when a new target position is given, the smoother works it's way there based
		// on the current motor position.
		{
			// Lock for smoothing
			std::lock_guard<std::mutex> lock(mMotorMutex);

			// Before we enable the smoothers, we need to update the current reference position.
			// This position is either the current motor position or current target position, if they motor is not available.
			int idx = 0;
			for (auto& smoother : mSmoothers)
			{
				if (mMotorMapping[idx] < mController->getSlaveCount())
				{
					nap::int32 actual_pos = mController->getActualPosition(mMotorMapping[idx]);
					smoother->setValue(static_cast<float>(actual_pos));
				}
				else
				{
					smoother->setValue(mMotorInput[idx]);
				}
				idx++;

			}
			mEnableSmoothing = true;
		}
	}


	bool MACAdapter::smoothingEnabled() const
	{
		return mEnableSmoothing;
	}


	void MACAdapter::setSmoothTime(float smoothTime)
	{
		mSmoothTimeLocal = math::max<float>(smoothTime, 0.001f);
	}


	float MACAdapter::getSmoothTime()
	{
		return mSmoothTimeLocal;
	}
}