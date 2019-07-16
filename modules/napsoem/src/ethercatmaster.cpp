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


	void EtherCATMaster::requestInitState()
	{
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