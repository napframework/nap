#pragma once

// External Includes
#include <nap/device.h>
#include <nap/numeric.h>
#include <nap/signalslot.h>
#include <future>
#include <atomic>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * ethercatmaster
	 */
	class NAPAPI EtherCATMaster : public Device
	{
		RTTI_ENABLE(Device)
	public:
		/**
		 * All available ethercat slave states
		 */
		enum class ESlaveState : uint16
		{
			None			= 0x00,				///< No valid state, slave probably lost.
			Init			= 0x01,				///< Init state
			PreOperational	= 0x02,				///< Pre operational state
			Boot			= 0x03,				///< Boot state
			SafeOperational = 0x04,				///< Safe operational state
			Operational		= 0x08,				///< Operational state
			Error			= 0x10,				///< Error state
			ACK				= 0x10				///< ACK state
		};

		// Stops the device
		virtual ~EtherCATMaster() override;

		/**
		 * Starts the master. Expects all slaves on the network to reach operational state.
		 * If one of the slaves fails to reach operational state the master is not started.
		 * Override the various operational stages to add your own SDO / PDO functionality during startup.
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the master. Kills error handling and processing threads.
		 * Tries to force all slaves into initialization mode. 
		 * onStop() is called after killing the processing and error handling tasks
		 * but before switching slaves to init mode.
		 */
		virtual void stop() override;

		/**
		 * @return if the master reached the operational stage and is therefore running.
		 */
		bool isRunning() const;
		
		/**
		 * Returns if a given slave is on-line (not lost). Index 0 refers to the first slave
		 * A slave that is on-line is not necessarily operations. Use getSlaveState() to get the actual status of a slave.
		 * @param index slave index, 0 = first slave. Does not perform an out of bounds check
		 * @return if a slave is on-line
		 */
		bool isOnline(int index) const;

		/**
		 * @return number of ether-cat slaves on the network.
		 */
		int getSlaveCount() const;

		/**
		 * @param index the index of the slave, 0 = first slave
		 * @return the states of a slave on the network.
		 */
		ESlaveState getSlaveState(int index) const;

		/**
		 * Updates the state of all slaves.
		 * @return lowest state of all read slave states.
		 */
		ESlaveState updateState();

		std::string mAdapter;			///< Property: 'Adapter' the name of the ethernet adapter to use. A list of available adapters is printed by the SOEM service on startup.
		int  mCycleTime = 1000;			///< Property: 'CycleTime' process cycle time in us. 

	protected:
		/**
		 * Returns a pointer to a ec_slavet (SOEM) struct.
		 * Note that index 0 refers to all slaves and index 1 to the first slave on the network.
		 * @return the slave at the given index. Does not perform any out of bounds check.
		 */
		void* getSlave(int index);

		//////////////////////////////////////////////////////////////////////////
		// Start / Stop
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Called after slave enumeration and initialization.
		 * All slaves are in  pre-operational state and can be addressed.
		 */
		virtual void onStart()	{ }

		/**
		 * Called after stopping the processing and error handling threads but
		 * before setting all slaves to init mode. Perform additional close up steps here.
		 */
		virtual void onStop()	{ }

		/**
		 * Returns the state associated with a specific slave.
		 * Index 1 refers to the first actual slave on the network.
		 * @param index slave index, 0 = master, 1 = first slave.
		 * @return state of a slave at the given index, no out of bounds check is performed.
		 */
		ESlaveState getState(int index) const;

		/**
		 * Returns if a slave is lost or not.
		 * Index 1 refers to the first actual slave on the network.
		 * @param index slave index, 0 = master, 1 = first slave.
		 * @return if a slave is lost (not visible on the network anymore)
		 */
		bool isLost(int index) const;

		//////////////////////////////////////////////////////////////////////////
		// Slave State Changes
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Called from the processing thread at a fixed interval defined by the cycle time property.
		 * Allows for real-time interaction with an ether-cat slave on the network by
		 * reading and writing to the slave's inputs or outputs.
		 * At this stage no SDO operations should take place, only operations that
		 * involve the access and modification of the PDO map.
		 * example:
		 *
		 *	ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(1));
		 *
		 * 	MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
		 *	mac_outputs->mOperatingMode = 2;
		 *	mac_outputs->mRequestedPosition = -1000000;
		 *	mac_outputs->mVelocity = 2700;
		 *	mac_outputs->mAcceleration = 360;
		 *	mac_outputs->mTorque = 341;
		 */
		virtual void onProcess() = 0;

		/**
		 * Called when a slave reaches the pre-operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * SDO communication is possible. No PDO communication.
		 * 
		 * Override this call to register a slave setup function, for example:
		 * void MyMaster::onPreOperational(void* slave, int index)
		 * {
		 *		reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC400_SETUP;
		 * }
		 * 
		 * You typically use the setup function to create your own custom PDO mapping.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array, 1 = first slave.
		 */
		virtual void onPreOperational(void* slave, int index)		{ }

		/**
		 * Called when a slave reaches the safe operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * PDO transmission is operational (slave sends data to master).
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array, 1 = first slave.
		 */
		virtual void onSafeOperational(void* slave, int index)		{ }

		/**
		 * Called when a slave reaches the operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * Drive fully operational, master responds to data via received PDO.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array, 1 = first slave.
		 */
		virtual void onOperational(void* slave, int index)			{ }

		//////////////////////////////////////////////////////////////////////////
		// Service Data Objects
		//////////////////////////////////////////////////////////////////////////

		/**
		 * SDO write, blocking. Single subindex or complete Access.
		 * It is not advised to write data when in operational mode.
		 * @param slave index of the slave, starting at 1. 
		 * @param index the index to write, 1 = first slave, 0 = master
		 * @param subindex the subindex to write
		 * @param CA false = single subindex, true = Complete Access, all subindexes written.
		 * @param psize size in bytes of parameter buffer
		 * @param p pointer to parameter buffer
		 */
		void sdoWrite(uint16 slave, uint16 index, uint8 subindex, bool ca, int psize, void* p);

		/**
		 * SDO read, blocking. Single subindex or complete Access.
		 * It is not advised to read data when in operational mode.
		 * @param slave index of the slave, starting at 1.
		 * @param index the index to read, 1 = first slave, 0 = master
		 * @param subindex the subindex to read
		 * @param CA false = single subindex, true = Complete Access, all subindexes written.
		 * @param psize size in bytes of parameter buffer, returns bytes read from SDO.
		 * @param p pointer to parameter buffer
		 */
		void sdoRead(uint16 slave, uint16 index, uint8 subindex, bool ca, int* psize, void* p);

		//////////////////////////////////////////////////////////////////////////
		// Slave State
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Blocking call to change the state of all slaves.
		 * Calls writeState and checkState internally.
		 * Waits 2 seconds (default timeout value) to ensure state actually changed.
		 * The actual state of all slaves is updated after this call.
		 * @param state the new state for all slaves.
		 * @param timeout in milliseconds, default = 2 seconds
		 * @return lowest state of all read slave states.
		 */
		ESlaveState requestState(ESlaveState state, int timeout = 2000);

		/** Write slave state, if slave = 0 then write to all slaves.
		 * The function does not check if the actual state is changed.
		 * @param index slave number, 0 = all slaves (master)
		 */
		void writeState(int index);

		/**
		 * Check the actual slave state, this is a blocking call.
		 * @param index slave index into SOEM ec_slave array, 0 = all slaves.
		 * @param state the state to check
		 * @param timeout timeout in ms
		 * @return requested state, or found state after timeout.
		 */
		ESlaveState checkState(int index, ESlaveState state, int timeout = 2000);

	private:
		char mIOmap[4096];
		int  mExpectedWKC = 0;
		std::future<void>	mProcessTask;						///< The background server thread
		std::future<void>	mErrorTask;							///< The background error checking thread
		std::atomic<bool>	mStopProcessing = { false };		///< If the task should be stopped
		std::atomic<bool>	mStopErrorTask = { false };			///< If the error task should be stopped
		std::atomic<int>	mActualWCK = { 0 };					///< Actual work counter
		std::atomic<bool>	mOperational = { false };			///< If the master is operational

		/**
		 * Real-time IO operations, executed on a different thread.
		 */
		void process();
		
		/**
		 * Automatic slave error checking and recovery.
		 */
		void checkForErrors();

		/**
		 * Process errors
		 * @param slaveGroup group of slaves to process error for
		 */
		void processErrors(int slaveGroup);
	};
}
