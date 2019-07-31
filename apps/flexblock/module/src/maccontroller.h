#pragma once

// External Includes
#include <nap/device.h>
#include <ethercatmaster.h>
#include <unordered_set>
#include <mutex>

namespace nap
{
	/**
	 * JVL CAM motor EthetCAT controller
	 * On initialization the controller creates a MACPosition container for every available slave on the network.
	 * This controller assumes that every slave is of type JVL MAC400-4500, using the MAC00-EC4 controller.
	 * When a slave reaches safe operational mode the current motor position is stored. This
	 * value is used to calculate the actual positional offset when the motor is in operational mode.
	 * This allows the motor to always aim for the actual position, even in between sessions
	 * and after loss of control due to program or driver failure.
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

		// Destructor
		virtual ~MACController();

		/**
		 * Set the position of a single motor. Does not perform an out of bounds check
		 * @param index motor index, 0 = first slave
		 * @param position the new motor target position
		 */
		void setPosition(int index, nap::uint32 position);

		/**
		 * Set the velocity in RPM of a single motor. Does not perform an out of bounds check.
		 * 1 RPM = 2.77056 counts/sample. Max velocity = 2000. Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave.
		 * @param velocity new motor velocity
		 */
		void setVelocity(int index, float velocity);

		/**
		 * Set the torque of a single motor. Does not perform an out of bounds check
		 * Accepted range = 0 - 300 %, where 100 % is nominal load and 300 % absolute peak load.
		 * Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave
		 * @param torque new motor torque
		 */
		void setTorque(int index, float torque);

		/**
		 * Set the desired nominal acceleration in RPM of a single motor. Does not perform an out of bounds check.
		 * 1000 RPM/s = 3.598133 counts/Sample². Negative values are clamped to 0.
		 * @param index motor index, 0 = first slave.
		 * @pram acceleration new motor acceleration
		 */
		void setAcceleration(int index, float acceleration);

		/**
		 * Converts a motor error into a human readable string
		 * @param error the motor error
		 * @return the string representation of the error 
		 */
		std::string errorToString(EErrorStat error);

		/**
		 * If the motor is in an invalid state due to 1 or more errors
		 * Always call this function first before sampling currently active errors
		 * Note that this is not the same as connectivity!
		 * @param index motor index, 0 = first slave
		 * @return if the motor is in an invalid state due
		 */
		bool hasErrors(int index) const;

		/**
		 * Returns a list of all errors associated with the motor
		 * Check for errors before calling this function!
		 * @param index motor index, 0 = first slave
		 * @return list of all available motor errors
		 */
		std::unordered_set<MACController::EErrorStat> getErrors(int index);

		bool mResetPosition = false;			///< Property: 'ResetPosition' if the motor position should be reset to the 'ResetPositionValue' before going into safe operational mode.
		nap::uint32 mResetPositionValue = 0;	///< Property: 'ResetPositionValue' the initial motor position value when reset position is turned on.
		nap::uint32 mRequestedPosition = 0;		///< Property: 'Position' requested motor position
		nap::uint32 mVelocity = 2700;			///< Property: 'Velocity' motor velocity
		nap::uint32 mAcceleration = 360;		///< Property: 'Acceleration' motor acceleration
		nap::uint32 mTorque = 341;				///< Property: 'Torque' motor torque

	protected:
		/**
		 * Describes various motor states
		 */
		enum class EMotorMode : nap::uint32
		{
			Passive		= 0,
			Position	= 2
		};

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
		void setMode(int index, EMotorMode mode);

	private:
		/**
		 * Simple struct to keep track of the motor position.
		 * The target position is set by the client, the 
		 * init position is read from the PDO when the master switches to safe operational mode.
		 */
		class MacOutputs
		{
		public:
			/**
			 * Set motor target position
			 * @param position new motor target position
			 */
			void setPosition(nap::uint32 position);

			/**
			 * Set motor velocity
			 * @param velocity new motor velocity
			 */
			void setVelocity(float velocity);

			/**
			 * Set motor torque
			 * @param torque new motor torque
			 */
			void setTorque(float torque);

			/**
			 * Set motor acceleration
			 * @param acceleration new motor acceleration
			 */
			void setAcceleration(float acceleration);

			std::atomic<nap::uint32>	mTargetPosition = { 0 };		///< New requested motor position
			std::atomic<nap::int32>		mInitPosition	= { 0 };		///< Initial motor position
			std::atomic<nap::uint32>	mVelocity		= { 0 };		///< Motor velocity
			std::atomic<nap::uint32>	mTorque			= { 0 };		///< Motor torque
			std::atomic<nap::uint32>	mAcceleration	= { 0 };		///< Motor acceleration
			std::atomic<bool>			mHasError		= { false };	///< If any errors are associated with the motor
			std::unordered_set<MACController::EErrorStat> mErrors;		///< List of all current errors
			std::mutex mErrorMutex;
		};
		
		/**
		 * If the current set of processed data contains the given error
		 * @param field contains all the processed error codes as a bits
		 * @param error the error to check for
		 */
		bool containsError(nap::uint32 field, EErrorStat error);

		std::vector<std::unique_ptr<MacOutputs>> mMotorParameters;		///< List of all current motor positions
	};
}
