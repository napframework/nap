// Local Includes
#include "maccontroller.h"

// External Includes
#include <soem/ethercat.h>
#include <nap/logger.h>
#include <mathutils.h>

// nap::maccontroller run time class definition 
RTTI_BEGIN_CLASS(nap::MACController)
	RTTI_PROPERTY("ResetPosition",			&nap::MACController::mResetPosition,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ResetPositionValue",		&nap::MACController::mResetPositionValue,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position",				&nap::MACController::mRequestedPosition,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Velocity",				&nap::MACController::mVelocity,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxVelocity",			&nap::MACController::mMaxVelocity,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Acceleration",			&nap::MACController::mAcceleration,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Torque",					&nap::MACController::mTorque,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VelocityGetRatio",		&nap::MACController::mVelocityGetRatio,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VelocitySetRatio",		&nap::MACController::mVelocitySetRatio,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::MACController::EErrorStat)
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::ThermalEnergyError,		"ThermalEnergyError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::FollowError,			"FollowError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::FunctionError,			"FunctionError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::BrakeResistorError,		"BrakeResistorError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::SoftwarePosError,		"SoftwarePosError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::TemperatureError,		"TemperatureError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::UnderVoltageError,		"UnderVoltageError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::OverVoltageError,		"OverVoltageError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::HighCurrentError,		"HighCurrentError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::SpeedError,				"SpeedError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::IndexError,				"IndexError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::ControlVoltageError,	"ControlVoltageError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::CommunicationsError,	"CommunicationsError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::CurrentLoopError,		"CurrentLoopError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::SlaveError,				"SlaveError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::AnyError,				"AnyError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::InitError,				"InitError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::FlashError,				"FlashError"),
	RTTI_ENUM_VALUE(nap::MACController::EErrorStat::SafeTorqueError,		"SafeTorqueError")
RTTI_END_ENUM

//////////////////////////////////////////////////////////////////////////
// Static callbacks
//////////////////////////////////////////////////////////////////////////

static int MAC_SETUP(uint16 slave)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
// MAC inputs / outputs
//////////////////////////////////////////////////////////////////////////

static constexpr float sVelCountSample		= 2.18435f;
static constexpr float sGetVelCountSample	= 0.134f;
static constexpr float sAccCountSample		= 3.598133f;
static constexpr float sTorqueNom			= 341.0f;
static constexpr nap::uint32 sNoErrorsCode	= 524304;

/**
 * IO Data sent to slave, acts as a memory lookup into PDO map
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mRequestedPosition;
	uint32_t	mVelocity;
	uint32_t	mAcceleration;
	uint32_t	mTorque;
	uint32_t	mAnalogueInput;
} MAC_400_OUTPUTS;


/**
 * IO Data received from slave, acts as a memory lookup into PDO map
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mActualPosition;
	int32_t		mActualVelocity;
	uint32_t	mAnalogueInput;
	uint32_t	mErrorStatus;
	int32_t		mActualTorque;
	uint32_t	mFollowError;
	uint32_t	mActualTemperature;
} MAC_400_INPUTS;


//////////////////////////////////////////////////////////////////////////
// MACController
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	MACController::~MACController()
	{
		mOutputs.clear();
		mInputs.clear();
	}


	void MACController::onPreOperational(void* slave, int index)
	{
		if (mResetPosition)
		{
			// Callback when going from pre-operational to safe operational
			reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC_SETUP;

			// Reset position if requested
			sdoWrite(index, 0x2012, 0x04, FALSE, sizeof(mResetPositionValue), &mResetPositionValue);

			// Force motor on zero.
			uint32_t control_word = 0;
			control_word |= 1UL << 6;
			control_word |= 0x0 << 8;
			sdoWrite(index, 0x2012, 0x24, FALSE, sizeof(control_word), &control_word);
		}
	}


	void MACController::onSafeOperational(void* slave, int index)
	{
		// Store initial motor position
		// This step is important as it ensures that the positional commands that are
		// given to the motor are relative to the initial position.
		assert(index <= getSlaveCount());
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)cslave->inputs;
		mOutputs[index - 1]->mInitPosition		= inputs->mActualPosition;
		mOutputs[index - 1]->mTargetPosition	= inputs->mActualPosition;

		// Force passive mode in safe operational mode
		setMode(index, EMotorMode::Passive);
	}


	void MACController::onProcess()
	{
		int slave_count = getSlaveCount();
		assert(mOutputs.size() == slave_count);
		assert(mInputs.size()  == slave_count);

		for (int i = 1; i <= slave_count; i++)
		{
			// Get slave to address
			ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(i));

			// Get associated motor output parameters
			std::unique_ptr<MacInputs>& motor_input = mInputs[i - 1];

			// Get inputs (data from slave)
			MAC_400_INPUTS* mac_inputs = (MAC_400_INPUTS*)slave->inputs;
			motor_input->mActualPosition = mac_inputs->mActualPosition;
			motor_input->mActualTorque = mac_inputs->mActualTorque;
			motor_input->mErrorStatus = mac_inputs->mErrorStatus;
			motor_input->mActualVelocity = mac_inputs->mActualVelocity;

			// Get associated motor output parameters
			std::unique_ptr<MacOutputs>& motor_output = mOutputs[i - 1];

			// Get relative motor position
			int32 req_position = motor_output->mTargetPosition - motor_output->mInitPosition;

			// Write info
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
			mac_outputs->mOperatingMode = static_cast<uint32_t>(EMotorMode::Position);
			mac_outputs->mRequestedPosition = req_position;
			mac_outputs->mVelocity = motor_output->mVelocityCNT;
			mac_outputs->mAcceleration = motor_output->mAccelerationCNT;
			mac_outputs->mTorque = motor_output->mTorqueCNT;
		}
	}


	void MACController::onStart()
	{
		mOutputs.clear();
		for (int i = 0; i < getSlaveCount(); i++)
		{
			// Create unique output
			std::unique_ptr<MacOutputs> new_output = std::make_unique<MacOutputs>(mMaxVelocity, mVelocitySetRatio);
			new_output->setPosition(mRequestedPosition);
			new_output->setAcceleration(static_cast<float>(mAcceleration));
			new_output->setVelocity(static_cast<float>(mVelocity));
			new_output->setTorque(static_cast<float>(mTorque));
			mOutputs.emplace_back(std::move(new_output));

			// Create unique input
			mInputs.emplace_back(std::make_unique<MacInputs>());
		}
		nap::Logger::info("%s: found %d slave(s)", this->mID.c_str(), this->getSlaveCount());
	}


	void MACController::onStop()
	{
		// Request new state
		EtherCATMaster::ESlaveState state = requestState(EtherCATMaster::ESlaveState::SafeOperational);
		if (state != EtherCATMaster::ESlaveState::SafeOperational)
			nap::Logger::warn("%s: not all slaves reached safe operational state", mID.c_str());

		// Set motor to passive mode
		for (int i = 1; i <= getSlaveCount(); i++)
			setMode(i, EMotorMode::Passive);
	}


	void MACController::setPosition(int index, nap::uint32 position)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setPosition(position);
	}


	uint32 MACController::getPosition(int index) const
	{
		assert(index < getSlaveCount());
		return mOutputs[index]->mTargetPosition;
	}

	nap::int32 MACController::getActualPosition(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualPosition();
	}


	void MACController::setVelocity(int index, float velocity)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setVelocity(velocity);
	}


	float MACController::getVelocity(int index) const
	{
		assert(index < getSlaveCount());
		return mOutputs[index]->getVelocity();
	}


	float MACController::getActualVelocity(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualVelocity(mVelocityGetRatio);
	}


	void MACController::setTorque(int index, float torque)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setTorque(torque);
	}


	float MACController::getActualTorque(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualTorque();
	}


	void MACController::setAcceleration(int index, float acceleration)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setAcceleration(acceleration);
	}


	std::string MACController::errorToString(EErrorStat error)
	{
		return RTTI_OF(MACController::EErrorStat).get_enumeration().value_to_name(error).to_string();
	}


	void MACController::errorToString(EErrorStat error, std::string& outString)
	{
		outString = RTTI_OF(MACController::EErrorStat).get_enumeration().value_to_name(error).to_string();
	}


	bool MACController::hasError(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->hasError();
	}


	void MACController::getErrors(int index, std::vector<MACController::EErrorStat>& outErrors) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getErrors(outErrors);
	}


	void MACController::setMode(int index, EMotorMode mode)
	{
		// Ensure the motor is in passive mode.
		uint32 new_mode = static_cast<nap::uint32>(mode);
		sdoWrite(index, 0x2012, 0x02, FALSE, sizeof(new_mode), &new_mode);
	}


	bool MacInputs::checkErrorBit(nap::uint32 field, MACController::EErrorStat error)
	{
		return (field & (1 << static_cast<uint32>(error))) > 0;
	}


	void MacOutputs::setPosition(nap::uint32 position)
	{
		mTargetPosition = position;
	}


	void MacOutputs::setVelocity(float velocity)
	{
		mVelocityRPM = math::clamp<float>(velocity, 0.0f, mMaxVelocityRPM);
		mVelocityCNT = static_cast<uint32>(mVelocityRPM * mRatio);
	}


	float MacOutputs::getVelocity() const
	{
		return mVelocityRPM;
	}


	void MacOutputs::setTorque(float torque)
	{
		float ptorqe = (math::clamp<float>(torque, 0.0f, 300.0f) / 100.0f) * sTorqueNom;
		mTorqueCNT = static_cast<uint32>(ptorqe);
	}


	void MacOutputs::setAcceleration(float acceleration)
	{
		float paccel = (static_cast<float>(math::max<float>(acceleration, 0.0f)) / 1000.0f) * sAccCountSample;
		mAccelerationCNT = static_cast<uint32>(paccel);
	}


	bool MacInputs::hasError() const
	{
		return checkErrorBit(mErrorStatus, MACController::EErrorStat::AnyError);
	}


	void MacInputs::getErrors(std::vector<nap::MACController::EErrorStat>& errors) const
	{
		// No errors
		errors.clear();
		if (checkErrorBit(mErrorStatus, MACController::EErrorStat::AnyError))
		{
			// Iterate over all the possible errors and check if bit is set
			rttr::enumeration error_enum = RTTI_OF(MACController::EErrorStat).get_enumeration();
			auto enum_values = error_enum.get_values();
			errors.reserve(enum_values.size());
			for (auto value : enum_values)
			{
				// Skip any error
				MACController::EErrorStat cur_e = static_cast<MACController::EErrorStat>(value.to_uint32());
				if (cur_e == MACController::EErrorStat::AnyError)
					continue;

				// Skip if the error bit is not set
				if (!checkErrorBit(mErrorStatus, cur_e))
					continue;

				// Add
				errors.emplace_back(cur_e);
			}
		}
	}


	float MacInputs::getActualVelocity(float velRatio) const
	{
		return static_cast<float>(mActualVelocity) / velRatio;
	}


	float nap::MacInputs::getActualTorque() const
	{
		return (static_cast<float>(mActualTorque) / sTorqueNom) * 100.0f;
	}
}
