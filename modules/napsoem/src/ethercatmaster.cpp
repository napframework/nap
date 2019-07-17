// Local Includes
#include "ethercatmaster.h"

// External Includes
#include <nap/logger.h>
#include <soem/ethercat.h>

// nap::ethercatmaster run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EtherCATMaster)
	RTTI_PROPERTY("Adapter",	&nap::EtherCATMaster::mAdapter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CycleTime",	&nap::EtherCATMaster::mCycleTime,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	EtherCATMaster::~EtherCATMaster()			{ }


	bool EtherCATMaster::start(utility::ErrorState& errorState)
	{
		// No tasks should be active at this point
		// The master shouldn't be in operation
		assert(!mOperational);

		// Try to initialize adapter
		if (!ec_init(mAdapter.c_str()))
		{
			errorState.fail("%s: no socket connection: %s", mID.c_str(), mAdapter.c_str());
			return false;
		}

		// Initialize all slaves, lib initialization is allowed to fail
		// It simply means no slaves are available
		if (ec_config_init(false) <= 0)
		{
			nap::Logger::warn("%s: no slaves found", mID.c_str());
			return true;
		}

		// Bind our thread and start sending / receiving slave data
		mStopProcessing = false;
		mProcessTask = std::async(std::launch::async, std::bind(&EtherCATMaster::process, this));

		// Bind our error thread, run until stopped
		mStopErrorTask = false;
		mErrorTask = std::async(std::launch::async, std::bind(&EtherCATMaster::checkForErrors, this));

		// All slaves should be in pre-op mode now
		ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
		for (int i = 1; i <= ec_slavecount; i++)
		{
			onPreOperational(&(ec_slave[i]), i);
		}

		// Configure io (SDO input / output) map
		ec_config_map(&mIOmap);
		ec_configdc();
		nap::Logger::info("%s: all slaves mapped", this->mID.c_str());

		// All slaves should be in safe-op mode now
		ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
		for (int i = 1; i <= ec_slavecount; i++)
		{
			onSafeOperational(&(ec_slave[i]), i);
		}

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
		{
			ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
		}

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

			// Request init state for all slaves and close connection
			stop();
			return false;
		}

		// All slaves successfully reached operational state
		nap::Logger::info("%s: all slaves reached operational state", mID.c_str());
		mOperational = true;
		for (int i = 1; i <= ec_slavecount; i++)
		{
			onOperational(&(ec_slave[i]), i);
		}

		// All good
		return true;
	}


	void EtherCATMaster::stop()
	{
		// Stop error reporting task
		if (mErrorTask.valid())
		{
			mStopErrorTask = true;
			mErrorTask.wait();
		}

		// Safety guard here, task is only created when 
		// at least 1 slave is found.
		if (mProcessTask.valid())
		{
			mStopProcessing = true;
			mProcessTask.wait();
		}

		// Make all slaves go to initialization stage
		requestInitState();

		// Close socket
		ec_close();

		// Reset work-counter and other variables
		mActualWCK		= 0;
		mExpectedWKC	= 0;
		mOperational	= false;
	}


	bool EtherCATMaster::isOperational() const
	{
		return mOperational;
	}


	int EtherCATMaster::getSlaveCount() const
	{
		return ec_slavecount;
	}


	nap::EtherCATMaster::ESlaveState EtherCATMaster::getSlaveState(int index) const
	{
		assert(index <= ec_slavecount);
		return static_cast<EtherCATMaster::ESlaveState>(ec_slave[index].state);
	}


	void* EtherCATMaster::getSlave(int index)
	{
		assert(index <= ec_slavecount);
		return &(ec_slave[index]);
	}


	void EtherCATMaster::onSafeOperational(void* slave, int index)
	{
		uint32_t new_pos = 0;
		ec_SDOwrite(index, 0x2012, 0x04, FALSE, sizeof(new_pos), &new_pos, EC_TIMEOUTSAFE);

		// Force motor on zero.
		uint32_t control_word = 0;
		control_word |= 1UL << 6;
		control_word |= 0x0 << 8;
		ec_SDOwrite(index, 0x2012, 0x24, FALSE, sizeof(control_word), &control_word, EC_TIMEOUTSAFE);
	}


	void EtherCATMaster::process()
	{
		// Keep running until stop is called
		while (!mStopProcessing)
		{
			if (mOperational)
			{
				// Process IO data
				onProcess();

				// Transmit processdata to slaves.
				ec_send_processdata();

				// Receive processdata from slaves.
				// Store the currently available work-counter
				mActualWCK = ec_receive_processdata(EC_TIMEOUTRET);

				// Sleep xUS
				osal_usleep(mCycleTime);
			}
		}
	}


	void EtherCATMaster::checkForErrors()
	{
		int currentgroup = 0;
		while (!mStopErrorTask)
		{
			// Operational stage not yet reached
			if (!mOperational)
			{
				osal_usleep(10000);
				continue;
			}

			// Work-count matches and we don't have to check slave states
			if (mActualWCK == mExpectedWKC && 
				!ec_group[currentgroup].docheckstate)
			{
				osal_usleep(10000);
				continue;
			}

			// one ore more slaves are not responding
			ec_group[currentgroup].docheckstate = FALSE;
			ec_readstate();
			for (int slave = 1; slave <= ec_slavecount; slave++)
			{
				if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
				{
					// Signal slave error in group
					ec_group[currentgroup].docheckstate = true;

					// In safe state with error bit set
					if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
					{
						nap::Logger::error("%s: slave %d is in SAFE_OP + ERROR, attempting ack", this->mID.c_str(), slave);
						ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
						ec_writestate(slave);
					}
					else if (ec_slave[slave].state == EC_STATE_SAFE_OP)
					{
						nap::Logger::warn("%s: slave %d is in SAFE_OP, change to OPERATIONAL", this->mID.c_str(), slave);
						onSafeOperational(&(ec_slave[slave]), slave);
						ec_slave[slave].state = EC_STATE_OPERATIONAL;
						ec_writestate(slave);
					}
					else if (ec_slave[slave].state > EC_STATE_NONE)
					{
						if (ec_reconfig_slave(slave, 500))
						{
							ec_slave[slave].islost = false;
							nap::Logger::info("%s: slave %d reconfigured", this->mID.c_str(), slave);
						}
					}
					else if (!ec_slave[slave].islost)
					{
						/* re-check state */
						ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
						if (ec_slave[slave].state == EC_STATE_NONE)
						{
							ec_slave[slave].islost = true;
							nap::Logger::error("%s: slave %d lost", this->mID.c_str(), slave);
						}
					}
				}
				
				// Slave is lost
				if (ec_slave[slave].islost)
				{
					if (ec_slave[slave].state == EC_STATE_NONE)
					{
						if (ec_recover_slave(slave, 500))
						{
							ec_slave[slave].islost = false;
							nap::Logger::info("%s: slave %d recovered", this->mID.c_str(), slave);
						}
					}
					else
					{
						ec_slave[slave].islost = false;
						nap::Logger::info("%s: slave %d found", this->mID.c_str(), slave);
					}
				}
			}

			// Check current group state, notify when all slaves resumed operational state
			if (!ec_group[currentgroup].docheckstate)
				nap::Logger::info("%s: all slaves resumed OPERATIONAL", this->mID.c_str());
			
			// Wait ten ms before checking again
			osal_usleep(10000);
		}
	}


	void EtherCATMaster::requestInitState()
	{
		nap::Logger::info("%s: Requesting init state for all slaves", mID.c_str());
		int chk = 200;
		ec_slave[0].state = EC_STATE_INIT;
		ec_writestate(0);
		while (chk-- && ec_slave[0].state != EC_STATE_INIT)
		{
			ec_statecheck(0, EC_STATE_INIT, EC_TIMEOUTSTATE);
		}
	}
}