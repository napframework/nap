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
	RTTI_PROPERTY("Smooth Using Velocity",	&nap::MACAdapter::mSmoothUsingVel,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Min Smooth Velocity",	&nap::MACAdapter::mSmoothMinVel,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Max Smooth Velocity",	&nap::MACAdapter::mSmoothMaxVel,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Smooth Time Min",		&nap::MACAdapter::mSmoothTimeMin,		nap::rtti::EPropertyMetaData::Default)
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

		// Apply lag
		applyLag(mMotorStepsInt, deltaTime);

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


	void MACAdapter::onStart()
	{
		mSetSteps = true;
	}


	void MACAdapter::onEnable(bool value)
	{
		mSetSteps = true;
	}


	void MACAdapter::applyLag(std::vector<float>& outSteps, double deltaTime)
	{
		// Simply copy input when lag is turned off
		std::lock_guard<std::mutex> lock(mMotorMutex);
		if (!mEnableSmoothing)
		{
			mMotorInput = mMotorStepsInt;
			return;
		}

		// Check if we need to update reference position of smoothers first
		if (mSetSteps)
		{
			for (auto i = 0; i < mSmoothers.size(); i++)
				mSmoothers[i]->setValue(outSteps[i]);

			mVelocity = 0.0f;
			mSetSteps = false;
		}

		// Scale time based on velocity
		float smooth_time = mSmoothTimeLocal;
		if (mSmoothUsingVel)
		{
			float min_target = math::min<float>(mSmoothTimeMin, mSmoothTimeLocal);
			smooth_time = math::fit<float>(mVelocity, mSmoothMinVel, mSmoothMaxVel, min_target, mSmoothTimeLocal);
		}

		// Update positions based on smoothed interpolation values
		assert(outSteps.size() == mSmoothers.size());

		mVelocity = 0.0f;
		for (int i = 0; i < mSmoothers.size(); i++)
		{
			mSmoothers[i]->mSmoothTime = smooth_time;
			outSteps[i] = mSmoothers[i]->update(outSteps[i], deltaTime);
			mVelocity += math::abs<float>(mSmoothers[i]->getVelocity());
		}

		// Calculate new current velocity and check if the velocity is falling
		mVelocity  /= static_cast<float>(mSmoothers.size());
		mMotorInput = mMotorStepsInt;
	}


	void MACAdapter::getMotorInput(std::vector<float>& outSteps)
	{
		std::lock_guard<std::mutex> lock(mMotorMutex);
		outSteps = mMotorInput;
	}


	void MACAdapter::getLag(std::vector<float>& outLag, std::vector<float>& outVel)
	{
		outLag.clear();
		outLag.resize(mSmoothers.size(), 0);
		outVel.clear();
		outVel.resize(mSmoothers.size(), 0);
		
		if (!mEnableSmoothing)
			return;

		std::lock_guard<std::mutex> lock(mMotorMutex);
		assert(mMotorInput.size() == mSmoothers.size());
		for (auto i = 0; i < mMotorInput.size(); i++)
		{
			outLag[i] = math::abs<float>(mSmoothers[i]->getTarget() - mMotorInput[i]);
			outVel[i] = mSmoothers[i]->getVelocity();
		}
	}


	void MACAdapter::enableSmoothing(bool value)
	{
		std::lock_guard<std::mutex> lock(mMotorMutex);
		mSetSteps = value;
		mEnableSmoothing = value;
	}


	bool MACAdapter::smoothingEnabled() const
	{
		return mEnableSmoothing;
	}


	void MACAdapter::setSmoothTime(float smoothTime)
	{
		mSmoothTimeLocal = math::max<float>(smoothTime, 0.01f);
	}


	float MACAdapter::getSmoothTime()
	{
		return mSmoothTimeLocal;
	}
}