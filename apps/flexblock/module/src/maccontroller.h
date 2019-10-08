#pragma once

// External Includes
#include <nap/device.h>
#include <ethercatmaster.h>
#include <unordered_set>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// MAC Controller
	//////////////////////////////////////////////////////////////////////////

	class MacOutputs;
	class MacInputs;

	/**
	 * JVL CAM motor EthetCAT controller
	 * This controller assumes that every slave is of type JVL MAC400-4500, using the MAC00-EC4 controller.
	 * The number of found slaves can be queried using: getSlaveCount(). The controller does not 
	 * impose a certain number of slaves. The code that interfaces with the controller must perform 
	 * an additional bounds check to ensure the right number of slaves are available on startup. 
	 * 
	 * Every slave interface function (setPosition, getActualPosition etc.) does not perform a bounds check.
	 * Make sure that before accessing the functions the index is not out of range. Other the device asserts.
	 *
	 * IMPORTANT: On Linux and OSX you must run the application that uses the controller as administrator, ie: with sudo privileges. 
	 * Failure to do so will result in a failure to start the device, and therefore the application!
	 *
	 * IMPORTANT: For the slave to function correctly make sure to ENABLE and SAVE in motor (using MACTalk):
	 *		- Position backup at powerloss (advanced tab)
	 *		- Resynchronize position after passive mode (advanced tab)
	 *		- Enable 8 cyclic R/W words (MAC00-EC -EtherCAT tab)
	 *		- Motor set velocity = 0 (MAC00-EC -EtherCAT tab)
	 *		- Startup operating mode = Passive + Change Actual Mode (main tab)
	 *
	 *	IMPORTANT: DISABLE and SAVE in motor (using MACTalk):
	 *		- Enable DSP402 drive profile (MAC00-EC -EtherCAT tab)
	 *
	 * IMPORTANT: Add '983040 - General Command' as the 7th write word in the cyclic data setup (MAC00-EC -EtherCAT tab).
	 *
	 * Some of these settings are recommended but the last one is REQUIRED (cyclic word), otherwise the motor will not be able
	 * to clear any errors and a runtime failure is likely to occur. If you are unable to alter the cyclic write data
	 * set the 'DisableErrorHandling' property to true, this will prevent writing into the 7th (non default) SDO 
	 * output register.
	 *
	 * To reset all the motors to a certain position at startup set the 'ResetPosition' property to true. This ensures
	 * that when the device is started the absolute and target motor position is set to the 'ResetPositionValue'. Alternatively:
	 * you can set the absolute motor position at runtime by calling resetPosition(). Without the 'ResetPosition' value set to
	 * true the target motor position is aligned with the current motor position, assuring the motor will not move on startup.
	 */
	class NAPAPI MACController : public EtherCATMaster
	{
		RTTI_ENABLE(EtherCATMaster)
	public:

		/**
		 * All available error codes
		 */
		enum class EErrorStat : uint32
		{
			ThermalEnergyError	= 0,	///< Set when the calculated thermal energy stored in the physical motor exceeds a limit.
			FollowError			= 1,	///< Set when the follow error gets larger than register 22
			FunctionError		= 2,	///< Set if the function error in Reg24, FNCERR, get slarger than Reg26
			BrakeResistorError  = 3,	///< Set when the calculated energy / temperature in the internal brake resistor(power dump) get dangerousl high.
			SoftwarePosError	= 7,	///< Set when one of the software position limits in Reg28 and Reg30 have been exceeded
			TemperatureError	= 8,	///< Set when the value in Reg29, DEGC, exceeds the value in Reg31
			UnderVoltageError	= 9,	///< Under voltage error
			OverVoltageError	= 11,	///< Over voltage error.
			HighCurrentError	= 12,	///< A much too high current was measured in one or more of the motor phases
			SpeedError			= 13,	///< The velocity was measured to be higher than a limit for an average of 16 samples
			IndexError			= 15,	///< The bit is set if an encoder error is detected
			ControlVoltageError = 17,	///< This error bit get set if the control voltage, normally at 24VDC, is below 12 V. The motor must be reset!
			CommunicationsError = 21,	///< Communications error (master or slave timeout with Modbus-Gear mode)
			CurrentLoopError	= 22,	///< Less than 2 mA was detected on the 4-20 mA input on the MAC00-P4 / P5 module for more than 100 ms
			SlaveError			= 23,	///< One or more error bits were set in an ERR_STAT reading from the Modbus slave or COMM_ERR
			AnyError			= 24,	///< single bit to make easier on PLCs to chefkc if the motor has any error bits set
			InitError			= 25,	///< Set if error was detected during motor startup that could prevent reliable operation
			FlashError			= 26,	///< An error was detected related to the internal flash memory during startup
			SafeTorqueError		= 27,	///< This bit gets set if the supervisor circuitry of the Safe Torque Off (STO)system detects an error.This will normally indicate an error in the electronics.
		};

		/**
		 * Describes various motor states
		 */
		enum class EMotorMode : nap::uint32
		{
			Passive		= 0,
			Velocity	= 1,
			Position	= 2
		};

		// Destructor
		virtual ~MACController();

		/**
		 * Set the position of a single motor. Does not perform an out of bounds check
		 * @param index motor index, 0 = first slave
		 * @param position the new motor target position
		 */
		void setPosition(int index, nap::int32 position);

		/**
		 * Returns the requested motor position for the given slave.
		 * @param index motor index, 0 = first slave
		 * @return the requested motor position
		 */
		int32 getPosition(int index) const;

		/**
		 * Returns the actual position of a single motor. Does not perform an out of bounds check
		 * @param index motor index, 0 = first slave
		 * @return the actual motor position
		 */
		nap::int32 getActualPosition(int index) const;

		/**
		 * Set the requested velocity in RPM of a single motor. Does not perform an out of bounds check.
		 * 1 RPM = approx 2.77056 counts/sample. Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave.
		 * @param velocity new motor velocity
		 */
		void setVelocity(int index, float velocity);

		/**
		 * @param index motor index, 0 = first slave.
		 * @return the requested velocity at the given motor index in RPM
		 */
		float getVelocity(int index) const;

		/**
		 * Returns the actual motor velocity in RPM. Does not perform an out of bounds check.
		 * 1 RPM = approx 2.77056 counts/sample.
		 * @param index motor index, 0 = first slave.
		 * @return the actual motor velocity in RPM
		 */
		float getActualVelocity(int index) const;

		/**
		 * Set the torque of a single motor. Does not perform an out of bounds check
		 * Accepted range = 0 - 300 %, where 100 % is nominal load and 300 % absolute peak load.
		 * Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave
		 * @param torque new motor torque
		 */
		void setTorque(int index, float torque);

		/**
		 * Returns the actual torque of a single motor. Does not perform an out of bounds check.
		 * @param index motor index, 0 = first slave.
		 * @return the actual torque of a single motor in the range of 0 - 300%
		 */
		float getActualTorque(int index) const;

		/**
		 * Set the desired nominal acceleration in RPM of a single motor. Does not perform an out of bounds check.
		 * 1000 RPM/s = approx 3.598133 counts/Sample². Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave.
		 * @pram acceleration new motor acceleration
		 */
		void setAcceleration(int index, float acceleration);

		/**
		 * Sets the motor mode to use. Does not perform an out of bounds check.
		 * @param index slave index, 0 = first slave.
		 */
		void setMode(int index, EMotorMode mode);

		/**
		 * Returns the actual motor mode.
		 * @param index motor index, 0 = first slave.
		 * @return the actual motor mode.
		 */
		EMotorMode getActualMode(int index) const;

		/**
		 * If the motor is in an invalid state due to 1 or more errors
		 * Always call this function first before sampling currently active errors
		 * Note that this is not the same as connectivity!
		 * @param index motor index, 0 = first slave
		 * @return if the motor is in an invalid state due
		 */
		bool hasError(int index) const;

		/**
		 * Returns a list of all errors associated with the motor
		 * Check for errors before calling this function!
		 * @param index motor index, 0 = first slave
		 * @return list of all available motor errors
		 */
		void getErrors(int index, std::vector<MACController::EErrorStat>& outErrors) const;

		/**
		 * Clears errors associated with a specific motor.
		 * When the error state of a motor is reset the motor tries to resume operational state.
		 * Errors are reset by default when the controller is started
		 * @param index the motor index to clear the errors for, 0 = first slave
		 */
		void clearErrors(int index);

		/**
		 * Sets the digital output pin to the requested value
		 * @param index motor index, 0 = first slave
		 * @param new value of output pin
		 */
		void setDigitalPin(int index, bool value);

		/**
		 * @param index motor index, 0 = first slave
		 * @return if the digital pin is active
		 */
		bool getDigitalPin(int index) const;

		/**
		 * Resets the position of all motors to the given value.
		 * This call needs to restart the device in order to update motor state.
		 * Do not call this function every frame!
		 * This overrides the current absolute and target position of the motor, ie:
		 * 'current_position = newPosition' and target_postion = newPosition.
		 * If this call fails the device will not be running.
		 * @param newPosition new absolute motor position
		 * @param error contains the error when resetting the position fails.
		 */
		bool resetPosition(nap::int32 newPosition, utility::ErrorState& error);

		/**
		 * Emergency stop call.
		 * Stops the device from running and puts all motors in a passive state.
		 */
		void emergencyStop();

		/**
		 * Converts a motor error into a human readable string
		 * @param error the motor error
		 * @return the string representation of the error
		 */
		static std::string errorToString(EErrorStat error);

		/**
		 * Converts a motor error into a human readable string
		 * @param error the motor error
		 * @param outString error converted to string
		 * @return the string representation of the error
		 */
		static void errorToString(EErrorStat error, std::string& outString);

		/**
		 * Converts a motor mode into a human readable string
		 * @param mode the motor state
		 * @return the string representation of the mode
		 */
		static std::string modeToString(EMotorMode mode);

		/**
		* Converts a motor mode into a human readable string
		* @param mode the motor mode
		* @param outString mode converted to string
		* @return the string representation of the mode
		*/
		static void modeToString(EMotorMode mode, std::string& outString);

		bool mResetPosition				= false;				///< Property: 'ResetPosition' if the motor position should be reset to the 'ResetPositionValue' before going into safe operational mode.
		nap::int32 mResetPositionValue	= 0;					///< Property: 'ResetPositionValue' the initial motor position value when reset position is turned on.
		nap::int32  mVelocity			= 0;					///< Property: 'Velocity' motor velocity in RPM
		nap::uint32 mAcceleration		= 360;					///< Property: 'Acceleration' motor acceleration in RPM
		nap::uint32 mTorque				= 341;					///< Property: 'Torque' motor torque from 0 to 300%
		nap::uint32 mMaxVelocity		= 4300;					///< Property: 'MaxVelocity' max allowed motor velocity
		EMotorMode	mMode				= EMotorMode::Position;	///< Property: 'Mode' the actual operating mode of the drive
		float mVelocityGetRatio			= 0.134f;				///< Property: 'VelocityGetRatio' Velocity counts / sample to RPM get ratio
		float mVelocitySetRatio			= 2.18435f;				///< Property: 'VelocitySetRatio' Velocity counts / sample to RPM set ratio
		uint mRecoveryTimeout			= 100;					///< Property: 'RecoveryTimeout' number of milliseconds to wait before recovery attempt (safe operational to operational)
		bool mDisableErrorHandling		= false;				///< Property: 'DisableErrorHandling' disables error handling when processing real-time data

		//
		nap::int32  mCalibrationVelocity = 0;					///< Property: 'Velocity' motor velocity in RPM
		nap::uint32 mCalibrationAcceleration = 360;					///< Property: 'Acceleration' motor acceleration in RPM
		nap::uint32 mCalibrationTorque = 341;					///< Property: 'Torque' motor torque from 0 to 300%
		nap::uint32 mCalibrationMaxVelocity = 4300;					///< Property: 'MaxVelocity' max allowed motor velocity
	protected:
		/**
		 * Called when a slave reaches the pre-operational stage on the network.
		 * Resets the motor position is requested.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onPreOperational(void* slave, int index) override;

		/**
		 * Called when a slave reaches the safe operational stage on the network.
		 * The controller has access to the input pdo and extracts the actual motor position.
		 * This value is used to calculate the actual position value that is used during the processing stage.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onSafeOperational(void* slave, int index) override;

		/**
		 * Called from the processing thread at a fixed interval defined by the cycle time property.
		 * Allows for real-time interaction with an ether-cat slave on the network by
		 * reading and writing to the slave's inputs or outputs.
		 * At this stage no SDO operations should take place, only operations that
		 * involve the access and modification of the PDO map.
		 */
		virtual void onProcess() override;

		/**
		 * Creates a map of all available mac motors.
		 * Occurs when all slaves have been enumerated and configured.
		 */
		virtual void onStart() override;

		/**
		 * Turns on passive mode for all motors
		 */
		virtual void onStop() override;

		/**
		 * Puts the motor in the requested mode using an sdo request.
		 * Do not call this when the motor is in operational mode!
		 * @param index slave index, 0 = all slaves, 1 = first slave
		 * @param mode the new motor mode
		 */
		void setModeSDO(int index, EMotorMode mode);

	private:
		std::vector<std::unique_ptr<MacOutputs>>	mOutputs;					///< List of all current motor positions
		std::vector<std::unique_ptr<MacInputs>>		mInputs;					///< List of all current motor positions
	};


	//////////////////////////////////////////////////////////////////////////
	// MAC motor outputs
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple struct to keep track of motor output parameters
	 * The target position is set by the client, the
	 * init position is read from the PDO when the master switches to safe operational mode.
	 */
	class MacOutputs
	{
		friend class MACController;
	public:
		/**
		 * Constructor
		 * @param maxVelocity the maximum allowed velocity in RPM 
		 * @param velRatio ratio used to convert counts per sample into RPM
		 */
		MacOutputs(float maxVelocity, float velRatio) :
			mMaxVelocityRPM(maxVelocity),
			mRatio(velRatio)	{ }

		/**
		 * Set motor target position
		 * @param position new motor target position
		 */
		void setTargetPosition(nap::int32 position);

		/**
		 * @return motor target position
		 */
		int32 getTargetPosition() const;

		/**
		 * Set motor velocity in RPM
		 * @param velocity new motor velocity in RPM
		 * @param maxVelocity the maximum allowed velocity in RPM
		 * @param velRatio ratio used to convert counts per sample into RPM
		 */
		void setTargetVelocity(float velocity);

		/**
		 * @return the currently set motor velocity in RPM.
		 */
		float getTargetVelocity() const;

		/**
		 * Set requested motor torque from 0 to 300 %
		 * @param torque new motor torque
		 */
		void setTargetTorque(float torque);

		/**
		 * Get requested motor torque in 0 - 300 %
		 */
		float getTargetTorque() const;

		/**
		 * Set motor acceleration
		 * @param acceleration new motor acceleration
		 */
		void setTargetAcceleration(float acceleration);

		/**
		 * Set the target motor mode
		 * @param mode the motor mode to use
		 */
		void setTargetMode(MACController::EMotorMode mode);

		/**
		 * Returns the requested motor mode
		 * @return the requested motor mode
		 */
		MACController::EMotorMode getTargetMode(MACController::EMotorMode mode) const;

		/**
		 * Sets the digital output pin to the requested value
		 * @param new value of output pin
		 */
		void setDigitalPin(bool value);

		/**
		 * @return if the digital pin is active
		 */
		bool getDigitalPin() const;

	private:
		std::atomic<nap::int32>		mTargetPosition = { 0 };	///< New requested motor position
		std::atomic<nap::int32>		mVelocityCNT = { 0 };		///< Motor velocity
		std::atomic<nap::uint32>	mTorqueCNT = { 0 };			///< Motor torque
		std::atomic<nap::uint32>	mAccelerationCNT = { 0 };	///< Motor acceleration
		std::atomic<nap::uint32>	mHardwareMode = { 9 };		///< Hardware operation mode
		std::atomic<nap::uint32>	mRunMode = { 0 };

		float						mRatio = 0.0f;				///< cnts / sample to RPM mapping
		float						mMaxVelocityRPM = 1000.0f;	///< Maximum allowed velocity in RPM
		float						mVelocityRPM = 0.0f;		///< Velocity in RPM
		float						mTorquePCT = 0.0f;			///< Torque in percentage
	};


	//////////////////////////////////////////////////////////////////////////
	// MAC motor inputs
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple struct to keep track of motor inputs (data received from slave).
 	 * This struct also keeps track of the motor error state
	 */
	class MacInputs
	{
		friend class MACController;
	public:

		/**
		 * @return actual motor position
		 */
		nap::int32 getActualPosition() const					{ return mActualPosition; }

		/**
		 * @param velRatio ratio used to convert cnts / sample into RPM
		 * @return actual motor velocity
		 */
		float getActualVelocity(float velRatio) const;

		/**
		 * @return actual torque in percentage from 0 - 300%
		 */
		float getActualTorque() const;

		/**
		 * @return actual motor operating mode
		 */
		MACController::EMotorMode getActualMode() const;

		/**
		 * @return if this motor is malfunctioning.
		 */
		bool hasError() const;

		/**
		 * Returns all errors associated with this motor
		 * Make sure to check if this motor has any errors before calling this function!
		 * @outErrors contains all the errors associated with this motor
		 */
		void getErrors(std::vector<MACController::EErrorStat>& outErrors) const;

		/**
		 * When called the clear errors flag is set.
		 * This causes the motor to erase all errors and resume (potential) operation.
		 */
		void clearErrors();

	private:
		std::atomic<nap::int32>	 mActualPosition	= { 0 };	///< Current motor position
		std::atomic<nap::int32>  mActualVelocity	= { 0 };	///< Current motor velocity
		std::atomic<nap::uint32> mErrorStatus		= { 0 };	///< Current error status
		std::atomic<nap::int32>	 mActualTorque		= { 0 };	///< Current motor torque
		std::atomic<nap::uint32> mActualMode		= { 0 };	///< Current Motor Mode
		std::atomic<bool>		 mClearErrors		= { true };	///< If the errors should be cleared

		/**
		 * If the current set of processed data contains the given error
		 * @param field contains all the processed error codes as a bits
		 * @param error the error to check for
		 */
		static bool checkErrorBit(nap::uint32 field, MACController::EErrorStat error);
	};
}
