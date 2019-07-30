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
	RTTI_PROPERTY("Acceleration",			&nap::MACController::mAcceleration,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Torque",					&nap::MACController::mTorque,				nap::rtti::EPropertyMetaData::Required)
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

static constexpr float sVelCountSample = 2.77056f;
static constexpr float sAccCountSample = 3.598133f;
static constexpr float sTorqueMax = 1023.0f;
static constexpr nap::uint32 sNoErrorsCode = 524304;

/**
 * Data sent to slave
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
 * Data received from slave
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mActualPosition;
	uint32_t	mActualVelocity;
	uint32_t	mAnalogueInput;
	uint32_t	mErrorStatus;
	uint32_t	mActualTorque;
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
		mMotorParameters.clear();
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
		assert(index <= getSlaveCount());
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)cslave->inputs;
		mMotorParameters[index - 1]->mInitPosition = inputs->mActualPosition;

		// Force passive mode in safe operational mode
		setMode(index, EMotorMode::Passive);
	}


	void MACController::onProcess()
	{
		int slave_count = getSlaveCount();
		assert(mMotorParameters.size() == slave_count);
		for (int i = 1; i <= slave_count; i++)
		{
			// Get slave to address
			ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(i));

			// Get inputs (data from slave)
			MAC_400_INPUTS* mac_inputs = (MAC_400_INPUTS*)slave->inputs;
			
			// Check if it contains an error, if so add it to the list of errors
			if (containsError(mac_inputs->mErrorStatus, MACController::EErrorStat::AnyError))
			{
				rttr::enumeration error_enum = RTTI_OF(MACController::EErrorStat).get_enumeration();
				for (auto value : error_enum.get_values())
				{
					// Skip any error
					MACController::EErrorStat cur_v = static_cast<MACController::EErrorStat>(value.to_uint32());
					if(cur_v == MACController::EErrorStat::AnyError)
						continue;

					// Check if the field contains this specific error
					if (containsError(mac_inputs->mErrorStatus, cur_v))
						mErrors.emplace(cur_v);
				}
			}

			// Get relative motor position
			std::unique_ptr<MacOutputs>& motor_parms = mMotorParameters[i - 1];
			int32 req_position = motor_parms->mTargetPosition - motor_parms->mInitPosition;

			// Write info
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
			mac_outputs->mOperatingMode = static_cast<uint32_t>(EMotorMode::Position);
			mac_outputs->mRequestedPosition = req_position;
			mac_outputs->mVelocity = motor_parms->mVelocity;
			mac_outputs->mAcceleration = motor_parms->mAcceleration;
			mac_outputs->mTorque = motor_parms->mTorque;
		}
	}


	void MACController::onStart()
	{
		mMotorParameters.clear();
		for (int i = 0; i < getSlaveCount(); i++)
		{
			std::unique_ptr<MacOutputs> new_output = std::make_unique<MacOutputs>();
			new_output->setPosition(mRequestedPosition);
			new_output->setAcceleration(static_cast<float>(mAcceleration));
			new_output->setVelocity(static_cast<float>(mVelocity));
			new_output->setTorque(static_cast<float>(mTorque));
			mMotorParameters.emplace_back(std::move(new_output));
		}
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
		mMotorParameters[index]->setPosition(position);
	}


	void MACController::setVelocity(int index, float velocity)
	{
		assert(index < getSlaveCount());
		mMotorParameters[index]->setVelocity(velocity);
	}


	void MACController::setTorque(int index, float torque)
	{
		assert(index < getSlaveCount());
		mMotorParameters[index]->setTorque(torque);
	}


	void MACController::setAcceleration(int index, float acceleration)
	{
		assert(index < getSlaveCount());
		mMotorParameters[index]->setAcceleration(acceleration);
	}


	void MACController::setMode(int index, EMotorMode mode)
	{
		// Ensure the motor is in passive mode.
		uint32 new_mode = static_cast<nap::uint32>(mode);
		sdoWrite(index, 0x2012, 0x02, FALSE, sizeof(new_mode), &new_mode);
	}


	bool MACController::containsError(nap::uint32 field, EErrorStat error)
	{
		return (field & (1 << static_cast<uint32>(error))) > 0;
	}


	void MACController::MacOutputs::setPosition(nap::uint32 position)
	{
		mTargetPosition = position;
	}


	void MACController::MacOutputs::setVelocity(float velocity)
	{
		float fvel = math::clamp<float>(velocity, 0.0f, 3000.0f) * sVelCountSample;
		mVelocity = static_cast<uint32>(fvel);
	}


	void MACController::MacOutputs::setTorque(float torque)
	{
		float ptorque = math::fit<float>(torque, 0.0f, 300.0f, 0.0f, sTorqueMax);
		mTorque = static_cast<uint32>(ptorque);
	}


	void MACController::MacOutputs::setAcceleration(float acceleration)
	{
		float paccel = (static_cast<float>(math::max<float>(acceleration, 0.0f)) / 1000.0f) * sAccCountSample;
		mAcceleration = static_cast<uint32>(paccel);
	}
}