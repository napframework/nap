// Local Includes
#include "maccontroller.h"

// External Includes
#include <soem/ethercat.h>
#include <nap/logger.h>
#include <mathutils.h>
#include <thread>

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

RTTI_BEGIN_ENUM(nap::MACController::EMotorMode)
	RTTI_ENUM_VALUE(nap::MACController::EMotorMode::Passive,				"Passive"),
	RTTI_ENUM_VALUE(nap::MACController::EMotorMode::Velocity,				"Velocity"),
	RTTI_ENUM_VALUE(nap::MACController::EMotorMode::Position,				"Position")
RTTI_END_ENUM

// nap::maccontroller run time class definition 
RTTI_BEGIN_CLASS(nap::MACController)
	RTTI_PROPERTY("ResetPosition",			&nap::MACController::mResetPosition,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("DisableErrorHandling",	&nap::MACController::mDisableErrorHandling,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ResetPositionValue",		&nap::MACController::mResetPositionValue,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Mode",					&nap::MACController::mMode,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Velocity",				&nap::MACController::mVelocity,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxVelocity",			&nap::MACController::mMaxVelocity,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Acceleration",			&nap::MACController::mAcceleration,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Torque",					&nap::MACController::mTorque,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VelocityGetRatio",		&nap::MACController::mVelocityGetRatio,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VelocitySetRatio",		&nap::MACController::mVelocitySetRatio,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RecoveryTimeout",		&nap::MACController::mRecoveryTimeout,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DigitalPinThreshold",	&nap::MACController::mDigitalPinThreshold,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InvertDigitalPin",		&nap::MACController::mInvertDigitalPin,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeDigitalPin",		&nap::MACController::mComputeDigitalPin,		nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("Calibration Velocity", &nap::MACController::mCalibrationVelocity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Calibration MaxVelocity", &nap::MACController::mCalibrationMaxVelocity, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Calibration Acceleration", &nap::MACController::mCalibrationAcceleration, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Calibration Torque", &nap::MACController::mCalibrationTorque, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// MAC inputs / outputs
//////////////////////////////////////////////////////////////////////////

static constexpr float sVelCountSample			= 2.18435f;
static constexpr float sGetVelCountSample		= 0.134f;
static constexpr float sAccCountSample			= 3.598133f;
static constexpr float sTorqueNom				= 341.0f;
static constexpr nap::uint32 sNoErrorsCode		= 524304;
static constexpr nap::uint32 sClearErrorCode	= 16777441;

/**
 * IO Data sent to slave, acts as a memory lookup into PDO map
 * Needs to match the Cyclic Data Write setup in MacTalk!
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mRequestedPosition;
	int32_t		mVelocity;
	uint32_t	mAcceleration;
	uint32_t	mTorque;
	uint32_t	mAnalogueInput;
	uint32_t	mModuleOutputs;
	uint32_t	mProgramCommands;
} MAC_400_OUTPUTS;


/**
 * IO Data received from slave, acts as a memory lookup into PDO map
 * Needs to match the Cyclic Data Read setup in MacTalk!
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
		mPositions.clear();
	}


	bool MACController::init(utility::ErrorState& errorState)
	{
		mComputePin = mComputeDigitalPin;
		return true;
	}


	void MACController::setPositionData(const std::vector<MacPosition>& newData)
	{
		// Copy current motor positions
		std::lock_guard<std::mutex> lock_guard(mPositionMutex);
		assert(newData.size() == getSlaveCount());
		mPositions = newData;
	}


	void MACController::getPositionData(std::vector<MacPosition>& outData)
	{
		std::lock_guard<std::mutex> lock_guard(mPositionMutex);
		outData = mPositions;
	}


	void MACController::setPosition(int index, nap::int32 position)
	{
		assert(index < getSlaveCount());
		std::lock_guard<std::mutex> lock_guard(mPositionMutex);
		mPositions[index].setTargetPosition(position);
	}


	int32 MACController::getPosition(int index) const
	{
		assert(index < getSlaveCount());
		std::lock_guard<std::mutex> lock_guard(mPositionMutex);
		return mPositions[index].mTargetPosition;
	}


	void MACController::onPreOperational(void* slave, int index)
	{
		// Control bits, very important on startup
		// The 5th bit ensures that when switching from passive to position mode
		// the internal motor position is updated! Without this bit set
		// following positional commands are relative and difficult to synchronize
		uint32_t control_bits = 0;
		control_bits |= 1UL << 5;		///< Restores position between modes

		// Reset motor position to new absolute value using control bits
		if (mResetPosition)
		{
			// Reset position if requested
			sdoWrite(index, 0x2012, 0x04, false, sizeof(mResetPositionValue), &mResetPositionValue);

			// Force motor to requested position.
			control_bits |= 1UL << 6;				//< Set 6th bit
			control_bits &= ~(1UL << 8);			//< Clear 8th bit
		}

		// Write control bits
		sdoWrite(index, 0x2012, 0x24, false, sizeof(control_bits), &control_bits);

	}


	void MACController::onSafeOperational(void* slave, int index)
	{
		// Ensure target position matches actual position
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)cslave->inputs;
		assert(index <= getSlaveCount());
		mPositions[index - 1].setTargetPosition(inputs->mActualPosition);

		// Motor controller has issues with synchronization when coming back-up from power failure.
		// Giving it some slack helps it getting into the right state.
		std::this_thread::sleep_for(std::chrono::milliseconds(mRecoveryTimeout));
	}


	void MACController::onProcess()
	{
		// Copy current motor positions
		std::vector<MacPosition> current_position_data;
		this->getPositionData(current_position_data);

		// Get number of slaves and ensure size of data matches
		int slave_count = getSlaveCount();
		assert(mOutputs.size() == slave_count);
		assert(mInputs.size()  == slave_count);
		assert(current_position_data.size() == slave_count);

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
			motor_input->mActualMode = mac_inputs->mOperatingMode;

			// Get associated motor output parameters
			std::unique_ptr<MacOutputs>& motor_output = mOutputs[i - 1];
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;

			// Get associated motor position data
			MacPosition& motor_position = mPositions[i - 1];

			// Write info
			mac_outputs->mOperatingMode = motor_output->mRunMode;
			mac_outputs->mRequestedPosition = motor_position.mTargetPosition;
			mac_outputs->mVelocity = motor_output->mVelocityCNT;
			mac_outputs->mAcceleration = motor_output->mAccelerationCNT;
			mac_outputs->mTorque = motor_output->mTorqueCNT;
			
			//////////////////////////////////////////////////////////////////////////

			// Calculate digital pin if requested
			if (mComputePin)
			{
				// Calculate if the pin should be set
				nap::int32 pos_delta = motor_position.mTargetPosition - motor_input->mActualPosition;
				bool set_pin = mInvertDigitalPin ? 
					pos_delta < (0 - mDigitalPinThreshold) :
					pos_delta > mDigitalPinThreshold;
				motor_output->setDigitalPin(0, set_pin);
			}

			// Store digital pin value
			mac_outputs->mModuleOutputs = motor_output->mModuleOutputs;

			//////////////////////////////////////////////////////////////////////////

			// Clear errors if requested
			if (motor_input->mClearErrors)
			{
				if (!mDisableErrorHandling)
					mac_outputs->mProgramCommands = sClearErrorCode;
				else
					nap::Logger::warn("%s: unable to clear errors, error handling disabled", mID.c_str());
				motor_input->mClearErrors = false;
			}
			else
			{
				if(!mDisableErrorHandling)
					mac_outputs->mProgramCommands = 0;
			}
		}
	}


	void MACController::onStart()
	{
		mOutputs.clear();
		mInputs.clear();
		mPositions.clear();

		for (int i = 0; i < getSlaveCount(); i++)
		{
			// Create unique output
			std::unique_ptr<MacOutputs> new_output = std::make_unique<MacOutputs>(static_cast<float>(mMaxVelocity), mVelocitySetRatio);
			new_output->setTargetAcceleration(static_cast<float>(mAcceleration));
			new_output->setTargetVelocity(static_cast<float>(mVelocity));
			new_output->setTargetTorque(static_cast<float>(mTorque));
			new_output->setTargetMode(mMode);
			mOutputs.emplace_back(std::move(new_output));

			// Create position outputs
			mPositions.emplace_back(MacPosition());

			// Create unique input
			mInputs.emplace_back(std::make_unique<MacInputs>());
		}
		nap::Logger::info("%s: found %d slave(s)", this->mID.c_str(), this->getSlaveCount());
	}


	void MACController::onStop()
	{
		// Only set mode when there are slaves managed on the network
		if (this->getSlaveCount() == 0)
			return;

		// Request new state
		EtherCATMaster::ESlaveState state = requestState(EtherCATMaster::ESlaveState::SafeOperational);
		if (state != EtherCATMaster::ESlaveState::SafeOperational)
			nap::Logger::warn("%s: not all slaves reached safe operational state", mID.c_str());

		// Set motor to passive mode
		for (int i = 1; i <= getSlaveCount(); i++)
			setModeSDO(i, EMotorMode::Passive);
	}

	nap::int32 MACController::getActualPosition(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualPosition();
	}


	void MACController::setVelocity(int index, float velocity)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setTargetVelocity(velocity);
	}


	float MACController::getVelocity(int index) const
	{
		assert(index < getSlaveCount());
		return mOutputs[index]->getTargetVelocity();
	}


	float MACController::getActualVelocity(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualVelocity(mVelocityGetRatio);
	}


	void MACController::setTorque(int index, float torque)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setTargetTorque(torque);
	}


	float MACController::getActualTorque(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualTorque();
	}


	void MACController::setAcceleration(int index, float acceleration)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setTargetAcceleration(acceleration);
	}


	void MACController::setMode(int index, EMotorMode mode)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setTargetMode(mode);
	}


	std::string MACController::errorToString(EErrorStat error)
	{
		return RTTI_OF(MACController::EErrorStat).get_enumeration().value_to_name(error).to_string();
	}


	void MACController::errorToString(EErrorStat error, std::string& outString)
	{
		outString = RTTI_OF(MACController::EErrorStat).get_enumeration().value_to_name(error).to_string();
	}


	std::string MACController::modeToString(EMotorMode mode)
	{
		return RTTI_OF(MACController::EMotorMode).get_enumeration().value_to_name(mode).to_string();
	}


	void MACController::modeToString(EMotorMode mode, std::string& outString)
	{
		outString = RTTI_OF(MACController::EMotorMode).get_enumeration().value_to_name(mode).to_string();
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


	void MACController::clearErrors(int index)
	{
		assert(index < getSlaveCount());
		return mInputs[index]->clearErrors();
	}


	void MACController::setDigitalPin(int index, int pinIndex, bool value)
	{
		assert(index < getSlaveCount());
		mOutputs[index]->setDigitalPin(pinIndex, value);
	}


	bool MACController::getDigitalPin(int index, int pinIndex) const
	{
		assert(index < getSlaveCount());
		return mOutputs[index]->getDigitalPin(pinIndex);
	}


	void MACController::setComputeDigitalPin(bool value)
	{
		mComputePin = value;
	}


	bool MACController::getComputeDigitalPin() const
	{
		return mComputePin;
	}


	bool MACController::resetPosition(nap::int32 newPosition, utility::ErrorState& error)
	{
		if (started())
		{
			this->stop();
		}
		mResetPosition = true;
		mResetPositionValue = newPosition;

		return this->start(error);
	}


	void MACController::emergencyStop()
	{
		if (started())
		{
			this->stop();
		}
	}


	void MACController::setModeSDO(int index, EMotorMode mode)
	{
		// Ensure the motor is in passive mode.
		uint32 new_mode = static_cast<nap::uint32>(mode);
		sdoWrite(index, 0x2012, 0x02, FALSE, sizeof(new_mode), &new_mode);
	}


	MACController::EMotorMode MACController::getActualMode(int index) const
	{
		assert(index < getSlaveCount());
		return mInputs[index]->getActualMode();
	}


	void MacInputs::clearErrors()
	{
		mClearErrors = true;
	}


	bool MacInputs::checkErrorBit(nap::uint32 field, MACController::EErrorStat error)
	{
		return (field & (1 << static_cast<uint32>(error))) > 0;
	}


	void MacOutputs::setTargetVelocity(float velocity)
	{
		mVelocityRPM = math::clamp<float>(velocity, mMaxVelocityRPM*-1.0f, mMaxVelocityRPM);
		mVelocityCNT = static_cast<int32>(mVelocityRPM * mRatio);
	}


	float MacOutputs::getTargetVelocity() const
	{
		return mVelocityRPM;
	}


	void MacOutputs::setTargetTorque(float torque)
	{
		mTorquePCT = math::clamp<float>(torque, 0.0f, 300.0f);
		mTorqueCNT = static_cast<uint32>((mTorquePCT / 100.0f) * sTorqueNom);
	}


	float MacOutputs::getTargetTorque() const
	{
		return mTorquePCT;
	}


	void MacOutputs::setTargetAcceleration(float acceleration)
	{
		float paccel = (static_cast<float>(math::max<float>(acceleration, 0.0f)) / 1000.0f) * sAccCountSample;
		mAccelerationCNT = static_cast<uint32>(paccel);
	}


	void MacOutputs::setTargetMode(MACController::EMotorMode mode)
	{
		mRunMode = static_cast<uint32>(mode);
	}


	MACController::EMotorMode MacOutputs::getTargetMode(MACController::EMotorMode mode) const
	{
		uint32 cmode = mRunMode;
		return static_cast<MACController::EMotorMode>(cmode);
	}


	nap::MACController::EMotorMode MacInputs::getActualMode() const
	{
		uint32 mode = mActualMode;
		return static_cast<MACController::EMotorMode>(mode);
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


	void MacOutputs::setDigitalPin(int pinIndex, bool value)
	{
		assert(pinIndex < 2);
		value ? mModuleOutputs |= 1UL << pinIndex : mModuleOutputs &= ~(1UL << pinIndex);
	}


	bool MacOutputs::getDigitalPin(int pinIndex) const
	{
		assert(pinIndex < 2);
		return ((mModuleOutputs >> pinIndex) & 1U) > 0;
	}
}
