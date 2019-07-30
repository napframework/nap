#pragma once

// External Includes
#include <nap/device.h>
#include <ethercatmaster.h>

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
		};
			
		std::vector<std::unique_ptr<MacOutputs>> mMotorParameters;		///< List of all current motor positions
	};
}
