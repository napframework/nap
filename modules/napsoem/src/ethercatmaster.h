/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * Ethercat Master Device. Derive from this class to implement your own ethercat master.
	 * 
	 * The Ethercat master finds and manages Ethercat slaves on the network.
	 * An Ethernet port is opened on start. The port to open is controlled by the 'Adapter' property. 
	 * Without a valid port the startup procedure will fail. 
	 * The SOEM service prints all available ports on startup of the service. 
	 * Pick one of the ports to discover slaves on the network.
	 *
	 * Override the various virtual functions to read / write to the slave SDO.
	 * Override the onProcess function to broadcast high priority control and status
	 * information using the slave PDO. onProcess() is called from a separate thread and
	 * should run on a frequency controlled by the 'CycleTime' property.
	 * It is your responsibility to time the process data transfers.
	 * Error reporting and slave recovery is also handled by this master on a separate thread.
	 *
	 * IMPORTANT: On Linux and OSX you must run the application that uses the 
	 * Ethercat master as administrator, ie: with sudo privileges. Failure
	 * to do so will result in a failure to start the device, and therefore
	 * the application.
	 *
	 * IMPORTANT: winpcap is required on windows. Download it here: https://www.winpcap.org/
	 * The SOEM module builds against the winpcap library but does not ship with the dll or driver.
	 * Installing winpcap should be enough. Administator priviliges are not required.
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

		// Constructor
		EtherCATMaster();

		// Destructor
		virtual ~EtherCATMaster() override;

		/**
		 * Starts the master. Expects all slaves on the network to reach operational state (when 'ForceOperational' is set to 'true').
		 * If no slaves are found the device does start but is not running: no background tasks are performed.
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
		 * When true ethercat data processing and error handling tasks are performed in the background.
		 * @return if ethercat data processing and error handling tasks are performed in the background
		 */
		bool isRunning() const;

		/**
		 * Returns if the device started successfully. 
		 * If no slaves are found on the network the master' started' but is not running. 
		 * If slaves are found on the network the master is 'running' if the start procedure succeeds.
		 * The ethernet port is open always open when started().
		 * @return if the master started. This does not mean the master is operational.
		 */
		bool started() const;
		
		/**
		 * Returns if a given slave is on-line (not lost).
		 * Index 1 refers to the first actual slave on the network, index 0 = all slaves.
		 * A slave that is on-line is not necessarily operational. Use getSlaveState() to get the actual status of a slave.
		 * @param index slave index, 1 = first slave. 0 = all slaves. Does not perform an out of bounds check
		 * @return if a slave is on-line
		 */
		bool isOnline(int index) const;

		/**
		 * @return number of ether-cat slaves on the network.
		 */
		int getSlaveCount() const;

		/**
		 * Returns the state of a slave on the network.
		 * Call readState() to update the state.
		 * Index 0 refers to the common state, index 1 is the first slave on the network.
		 * @param index the index of the slave. 1 = first slave on the network.
		 * @return the states of a slave on the network.
		 */
		ESlaveState getSlaveState(int index) const;

		/**
		 * @param index the index of the slave. 1 = first slave on the network.
		 * @return name of a slave.
		 */
		 std::string getSlaveName(int index);

		/**
		 * Reads the state of all slaves on the network and stores the result.
		 * @return lowest state of all read slave states.
		 */
		ESlaveState readState();

		bool mForceOperational = false;		///< Property: 'ForceOperational' if all slaves need to reach operational state during startup.
		std::string mAdapter;				///< Property: 'Adapter' the name of the ethernet adapter to use. A list of available adapters is printed by the SOEM service on startup.
		int mProcessCycleTime	= 1000;		///< Property: 'ProcessCycleTime' process cycle time in us. 1000us = 1ms
		int mErrorCycleTime		= 40000;	///< Property: 'ErrorCycleTime' error checking cycle time in us. 1000us = 1ms
		int mRecoveryTimeout	= 500;		///< Property: 'RecoveryTimeout' given time (in us) for a slave to recover. 1000us = 1ms
		int mProcessTimeout		= 2000;		///< Property: 'ProcessTimeout' time (in us) the master is given to process all slave input / output.

	protected:

		//////////////////////////////////////////////////////////////////////////
		// Slave State Changes
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Called after slave enumeration and initialization.
		 * All slaves are in  PRE-Operational state and can be addressed.
		 */
		virtual void onStart() { }

		/**
		 * Called after stopping the processing and error handling threads but
		 * before setting all slaves to init mode. Perform additional close up steps here.
		 */
		virtual void onStop() { }

		/**
		 * Called from the processing thread, override in derived class.
		 * Allows for real-time interaction with an ether-cat slave on the network by
		 * reading and writing to the slave's inputs or outputs.
		 * It is your responsibility to handle all timing and master - slave synchronization.
		 * At this stage no SDO operations should take place, only operations that
		 * involve the access and modification of the PDO map.
		 */
		virtual void onProcess() = 0;

		/**
		 * Called by the main thread when the processing task should be stopped.
		 */
		virtual void onStopProcessing() = 0;

		/**
		 * Called when a slave reaches the pre-operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * SDO communication is possible. No PDO communication.
		 * 
		 * Override this call to register a slave setup function, for example:
		 * void MyMaster::onPreOperational(void* slave, int index)
		 * {
		 *		reinterpret_cast<ec_slavet*>(slave)->PO2SOconfigx = &MAC400_SETUP;
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
		 * @param ca false = single subindex, true = Complete Access, all subindexes written.
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
		 * @param ca false = single subindex, true = Complete Access, all subindexes written.
		 * @param psize size in bytes of parameter buffer, returns bytes read from SDO.
		 * @param p pointer to parameter buffer
		 * @return work counter
		 */
		int sdoRead(uint16 slave, uint16 index, uint8 subindex, bool ca, int* psize, void* p);

		//////////////////////////////////////////////////////////////////////////
		// Slave State
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Returns a pointer to a ec_slavet (SOEM) struct.
		 * Note that index 0 refers to all slaves and index 1 to the first slave on the network.
		 * @return the slave at the given index. Does not perform any out of bounds check.
		 */
		void* getSlave(int index);

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
		 * Check actual slave state. This is a blocking function.
		 * @param index slave index into SOEM ec_slave array, 0 = all slaves.
		 * @param state the requested state
		 * @param timeout timeout in ms
		 * @return requested state, or found state after timeout.
		 */
		ESlaveState stateCheck(int index, ESlaveState state, int timeout = 2000);

		//////////////////////////////////////////////////////////////////////////
		// Process Data
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Send process data to all slaves.
		 */
		void sendProcessData();

		/** 
		 * Receive process data from slaves. Should be called from onProcess.
		 * Received datagrams are recombined with the process data with help from the stack.
		 * If a datagram contains input process data it copies it to the process data structure.
		 * @param timeout Timeout in us.
		 * @return Work counter.
		 */
		int receiveProcessData(int timeout);

		//////////////////////////////////////////////////////////////////////////
		// Distributed Clock
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Returns if a specific slave has a distributed clock. 
		 * When index 0 is queried, the return value indicates if at least 1 slave has a distributed clock.
		 * If 1 slave has a distributed clock the master can synchronize to it.
		 * @param slave the slave to check, 0 = all slaves.
		 * @return if the slave has distributed clock capability
		 */
		bool hasDistributedClock(int slave);

		/**
		 * Returns if at least 1 slave has a distributed clock.
		 * If that's the case the system can synchronize to it.
		 * @return if at least 1 slave has a distributed clock
		 */
		bool hasDistributedClock();

		/**
		 * @return distributed clock
		 */
		int64 getDistributedClock();

		//////////////////////////////////////////////////////////////////////////
		// Context
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Returns a pointer to the ecx_contextt (SOEM) struct.
		 * @return a pointer to the ecx_contextt (SOEM) struct.
		 */
		 void* getContext();

	private:
		char mIOmap[4096];
		int  mExpectedWKC = 0;
		std::future<void>	mProcessTask;							///< The background server thread
		std::future<void>	mErrorTask;								///< The background error checking thread
		std::atomic<bool>	mStopErrorTask = { false };			///< If the error task should be stopped
		std::atomic<int>	mActualWCK = { 0 };					///< Actual work counter
		std::atomic<bool>	mOperational = { false };			///< If the master is operational
		std::atomic<bool>	mRunning = { false };				///< If the processing thread is running
		bool				mStarted = false;						///< If the master started, this does not mean it's operational
		void*				mContext = nullptr;						///< Ethercat (SOEM) context of type ecx_contextt

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
		void processErrors();

		/**
		 * Creates an error message if a slave is not in the required state.
		 * All slaves that are not in the required state are added.
		 * This call reads the state before checking for errors.
		 * @param requiredState required state for the slave to be in.
		 * @param outError contains the error message if slave is not in that state
		 * @return if there are any error messages
		 */
		void getStatusMessage(ESlaveState requiredState, utility::ErrorState& outError);
	};
}
