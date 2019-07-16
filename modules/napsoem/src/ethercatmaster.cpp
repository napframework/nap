// Local Includes
#include "ethercatmaster.h"

// External Includes
#include <nap/logger.h>
#include <soem/ethercat.h>

// nap::ethercatmaster run time class definition 
RTTI_BEGIN_CLASS(nap::EtherCATMaster)
	RTTI_PROPERTY("Adapter", &nap::EtherCATMaster::mAdapter, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mRequestedPosition;
	uint32_t	mVelocity;
	uint32_t	mAcceleration;
	uint32_t	mTorque;
	uint32_t	mAnalogueInput;
} MAC_400_OUTPUTS;

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


namespace nap
{
	EtherCATMaster::~EtherCATMaster()			{ }


	bool EtherCATMaster::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool EtherCATMaster::start(utility::ErrorState& errorState)
	{
		// Try to initialize adapter
		if (!ec_init(mAdapter.c_str()))
		{
			errorState.fail("%s: no socket connection: %s", mID.c_str(), mAdapter.c_str());
			return false;
		}

		// Initialize all slaves, lib initialization is allowed to fail
		// It simply means no slaves are available
		if (ec_config_init(false) <= 0)
			return true;

		// TODO: install callback here

		// Configure io (SDO input / output) map
		ec_config_map(&mIOmap);
		ec_configdc();
		nap::Logger::info("%s: all slaves mapped", this->mID.c_str());

		// All slaves should be in safe-op mode now
		ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
		mExpectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
		nap::Logger::info("%s: calculated workcounter: %d", mID.c_str(), mExpectedWKC);

		// send one valid process data to all slaves to wake them up
		ec_slave[0].state = EC_STATE_OPERATIONAL;
		ec_send_processdata();
		ec_receive_processdata(EC_TIMEOUTRET);

		// Bind our thread
		mTask = std::async(std::launch::async, std::bind(&EtherCATMaster::run, this));

		// request Operational state for all slaves
		ec_writestate(0);

		// Wait until all slaves are in operational state
		int chk = 200;
		while (chk-- && ec_slave[0].state != EC_STATE_OPERATIONAL)
			ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);

		// Ensure all slaves are in operational state
		if (ec_slave[0].state != EC_STATE_OPERATIONAL)
		{
			errorState.fail("%s: not all slaves reached operational state!", mID.c_str());
			ec_readstate();
			for (int i = 0; i < ec_slavecount; i++)
			{
				if (ec_slave[i].state == EC_STATE_OPERATIONAL)
					continue;

				errorState.fail("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
					i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
			}

			// Request init state for all slaves
			requestInitState();
			return false;
		}

		// All slaves successfully reached operational state
		nap::Logger::info("%s: all slaves reached operational state", mID.c_str());

		// All good
		return true;
	}


	void EtherCATMaster::stop()
	{
		// Make all slaves go to initialization stage
		requestInitState();

		// Close socket
		ec_close();
	}


	int EtherCATMaster::getSlaveCount() const
	{
		return ec_slavecount;
	}


	void EtherCATMaster::run()
	{
		while (!mStopRunning)
		{
			// Read info
			MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)ec_slave[1].inputs;
			inputs->mErrorStatus;

			// Write info
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)ec_slave[1].outputs;
			mac_outputs->mOperatingMode = 2;
			mac_outputs->mRequestedPosition = -1000000;
			mac_outputs->mVelocity = 2700;
			mac_outputs->mAcceleration = 360;
			mac_outputs->mTorque = 341;

			ec_send_processdata();
			mActualWCK = ec_receive_processdata(EC_TIMEOUTRET);

			// Sleep for 1 millisecond
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}


	void EtherCATMaster::requestInitState()
	{
		assert(mTask.valid());
		mStopRunning = true;
		mTask.wait();

		nap::Logger::info("%s: Requesting init state for all slaves", mID.c_str());
		int chk = 200;
		ec_slave[0].state = EC_STATE_INIT;
		ec_writestate(0);
		while (chk-- && ec_slave[0].state != EC_STATE_INIT)
		{
			ec_statecheck(0, EC_STATE_INIT, 50000);
		}
	}
}