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
		 * All available ethercast slave states
		 */
		enum class ESlaveState : uint
		{
			None			= 0x00,				///< No valid state
			Init			= 0x01,				///< Init state
			PreOperational	= 0x02,				///< Pre operational state
			Boot			= 0x03,				///< Boot state
			SafeOperational = 0x04,				///< Safe operational state
			Operational		= 0x08,				///< Operational state
			Error			= 0x10				///< Error state
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
		 * Stops the master.
		 */
		virtual void stop() override;

		/**
		 * @return if the master reached the operational stage and is therefore running
		 */
		bool isOperational() const;

		/**
		 * @return number of ether-cat slaves on the network.
		 */
		int getSlaveCount() const;

		/**
		 * Returns the state associated with a specific slave. 
		 * Note that index 0 refers to the first slave on the network.
		 * @return state of a slave at the given index, no out of bounds check is performed.
		 */
		ESlaveState getSlaveState(int index) const;

		std::string mAdapter;			///< Property: 'Adapter' the name of the ethernet adapter to use. A list of available adapters is printed by the SOEM service on startup.
		int  mCycleTime = 1000;			///< Property: 'CycleTime' process cycle time in us. 

	protected:
		/**
		 * Returns a pointer to a ec_slavet (SOEM) struct.
		 * Note that index 0 refers to all slaves and index 1 to the first slave on the network.
		 * @return the slave at the given index. Does not perform any out of bounds check.
		 */
		void* getSlave(int index);

		/**
		 * Called when a slave reaches the pre-operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * SDO communication is possible. No PDO communication.
		 * 
		 * Override this call to register a slave setup function, for example:
		 * void MyMaster::onPreOperational(void* slave)
		 * {
		 *		reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC400_SETUP;
		 * }
		 * 
		 * You typically use the setup function to create your own custom PDO mapping.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onPreOperational(void* slave, int index);

		/**
		 * Called when a slave reaches the safe operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * PDO transmission is operational (slave sends data to master).
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onSafeOperational(void* slave, int index);

		/**
		 * Called when a slave reaches the operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * Drive fully operational, master responds to data via received PDO.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onOperational(void* slave, int index)			{ }

		/**
		 * Called from the processing thread at a fixed interval defined by the cycle time property.
		 * Allows for real-time interaction with an ether-cat slave on the network by 
		 * reading and writing to the slave's inputs or outputs.
		 * At this stage no SDO operations should take place, only operations that
		 * involve the access and modification of the PDO map.
		 */
		virtual void onProcess();

	private:
		char mIOmap[4096];
		int  mExpectedWKC = 0;
		std::future<void>	mProcessTask;								///< The background server thread
		std::future<void>	mErrorTask;							///< The background error checking thread
		std::atomic<bool>	mStopProcessing = { false };			///< If the task should be stopped
		std::atomic<bool>	mStopErrorTask = { false };			///< If the error task should be stopped
		std::atomic<int>	mActualWCK = { 0 };					///< Actual work counter
		std::atomic<bool>	mOperational = { false };			///< If the master is operational

		void process();
		
		void checkForErrors();

		void requestInitState();
	};
}
