/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "ethercatmaster.h"

// External Includes
#include <nap/logger.h>
#include <soem/ethercat.h>

// nap::ethercatmaster run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EtherCATMaster)
	RTTI_PROPERTY("ForceOperational",	&nap::EtherCATMaster::mForceOperational,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Adapter",			&nap::EtherCATMaster::mAdapter,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ProcessCycleTime",	&nap::EtherCATMaster::mProcessCycleTime,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ErrorCycleTime",		&nap::EtherCATMaster::mErrorCycleTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ProcessTimeout",		&nap::EtherCATMaster::mProcessTimeout,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RecoveryTimeout",	&nap::EtherCATMaster::mRecoveryTimeout,		nap::rtti::EPropertyMetaData::Default)
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
		assert(!mStarted);

		// Try to initialize adapter
		if (!ec_init(mAdapter.c_str()))
		{
			errorState.fail("%s: no socket connection: %s", mID.c_str(), mAdapter.c_str());
			return false;
		}

		// Initialize all slaves, lib initialization is allowed to fail
		// Simply means no slaves are available on the network
		if (ec_config_init(false) <= 0)
		{
			nap::Logger::warn("%s: no slaves found", mID.c_str());
			mStarted = true;
			return true;
		}

		// Call init
		onStart();

		// All slaves should be in pre-op mode now
		checkState(0, ESlaveState::PreOperational, EC_TIMEOUTSTATE / 1000);
		for (int i = 1; i <= ec_slavecount; i++)
		{
			onPreOperational(&(ec_slave[i]), i);
		}

		// Configure io (SDO input / output) map
		ec_config_map(&mIOmap);
		ec_configdc();
		nap::Logger::info("%s: all slaves mapped", this->mID.c_str());

		// All slaves should be in safe-op mode now
		checkState(0, ESlaveState::SafeOperational, EC_TIMEOUTSTATE / 1000);

		// Send some data to make slaves happy
		ec_send_processdata();
		ec_receive_processdata(mProcessTimeout);

		// Notify listeners
		for (int i = 1; i <= ec_slavecount; i++)
		{
			onSafeOperational(&(ec_slave[i]), i);
		}

		// Bind our thread and start sending / receiving slave data
		// We do that here to ensure a fast transition from safe to operational.
		mStopProcessing = false;
		mProcessTask = std::async(std::launch::async, std::bind(&EtherCATMaster::process, this));

		// Bind our error thread, run until stopped
		// We do that here to ensure a fast transition from safe to operational.
		mStopErrorTask = false;
		mErrorTask = std::async(std::launch::async, std::bind(&EtherCATMaster::checkForErrors, this));

		// Calculate slave work counter, used to check in the error loop if slaves got lost
		mExpectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
		nap::Logger::info("%s: calculated work-counter: %d", mID.c_str(), mExpectedWKC);

		// request Operational state for all slaves, give it 10 seconds
		EtherCATMaster::ESlaveState state = requestState(ESlaveState::Operational, 10000);
		updateState();

		// If not all slaves reached operational state display errors and 
		// bail if operational state of all slaves is required on startup
		mOperational = true;
		if (state != EtherCATMaster::ESlaveState::Operational)
		{
			std::string fail_msg = utility::stringFormat("%s: not all slaves reached operational state!", mID.c_str());
			nap::Logger::warn(fail_msg);
			errorState.fail(fail_msg);
			for (int i = 1; i <= ec_slavecount; i++)
			{
				if (ec_slave[i].state == static_cast<uint16>(EtherCATMaster::ESlaveState::Operational))
					continue;

				// Get error code
				std::string fail_state = utility::stringFormat("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
					i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));

				nap::Logger::warn(fail_state);
				errorState.fail(fail_state);
			}

			if (mForceOperational)
			{
				stop();
				return false;
			}
		}
		else
		{
			nap::Logger::info("%s: all slaves reached operational state", mID.c_str());
		}

		// All slaves successfully reached operational state
		for (int i = 1; i <= ec_slavecount; i++)
		{
			if (ec_slave[i].state == static_cast<uint16>(EtherCATMaster::ESlaveState::Operational))
				onOperational(&(ec_slave[i]), i);
		}

		mStarted = true;
		return true;
	}


	void EtherCATMaster::stop()
	{
		// Safety guard here, task is only created when at least 1 slave is found
		// Stop error reporting task
		if (mErrorTask.valid())
		{
			mStopErrorTask = true;
			mErrorTask.wait();
		}

		// Safety guard here, task is only created when at least 1 slave is found.
		// Stop processing task
		if (mProcessTask.valid())
		{
			mStopProcessing = true;
			mProcessTask.wait();
		}

		// Call stop and request init state for all slaves
		onStop();
		if (getSlaveCount() > 0)
		{
			// Make all slaves go to initialization stage
			if (requestState(ESlaveState::Init) != ESlaveState::Init)
			{
				nap::Logger::warn("%s: not all slaves reached init state", mID.c_str());
			}
		}

		// Close socket
		ec_close();	

		// Reset work-counter and other variables
		mActualWCK		= 0;
		mExpectedWKC	= 0;
		mOperational	= false;
		mStarted		= false;
	}


	bool EtherCATMaster::isRunning() const
	{
		return mOperational;
	}


	bool EtherCATMaster::started() const
	{
		return mStarted;
	}


	bool EtherCATMaster::isOnline(int index) const
	{
		return !isLost(index + 1);
	}


	int EtherCATMaster::getSlaveCount() const
	{
		return ec_slavecount;
	}


	nap::EtherCATMaster::ESlaveState EtherCATMaster::getSlaveState(int index) const
	{
		return getState(index + 1);
	}


	nap::EtherCATMaster::ESlaveState EtherCATMaster::updateState()
	{
		return static_cast<ESlaveState>(ec_readstate());
	}


	nap::EtherCATMaster::ESlaveState EtherCATMaster::getState(int index) const
	{
		assert(index <= ec_slavecount);
		uint16 cstate = ec_slave[index].state;
		return cstate > static_cast<uint16>(EtherCATMaster::ESlaveState::Operational) ? 
			EtherCATMaster::ESlaveState::Error : 
			static_cast<EtherCATMaster::ESlaveState>(cstate);
	}


	bool EtherCATMaster::isLost(int index) const
	{
		assert(index <= ec_slavecount);
		return ec_slave[index].islost > 0;
	}


	void* EtherCATMaster::getSlave(int index)
	{
		assert(index <= ec_slavecount);
		return &(ec_slave[index]);
	}


	void EtherCATMaster::sdoWrite(uint16 slave, uint16 index, uint8 subindex, bool ca, int psize, void *p)
	{
		ec_SDOwrite(slave, index, subindex, ca, psize, p, EC_TIMEOUTRXM);
	}


	void EtherCATMaster::sdoRead(uint16 slave, uint16 index, uint8 subindex, bool ca, int* psize, void* p)
	{
		ec_SDOread(slave, index, subindex, ca, psize, p, EC_TIMEOUTRXM);
	}

	void EtherCATMaster::process()
	{
		// Keep running until stop is called
		while (!mStopProcessing)
		{
			if (!mOperational)
			{
				osal_usleep(mProcessCycleTime);
				continue;
			}

			// Process
			onProcess();

			// Transmit process-data to slaves and store actual work counter
			ec_send_processdata();
			mActualWCK = ec_receive_processdata(mProcessTimeout);
		
			// Sleep xUS
			osal_usleep(mProcessCycleTime);
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
				osal_usleep(mErrorCycleTime);
				continue;
			}

			// Work-count matches and we don't have to check slave states
			if (mActualWCK == mExpectedWKC && 
				!ec_group[currentgroup].docheckstate)
			{
				osal_usleep(mErrorCycleTime);
				continue;
			}
			
			// One or more slaves are not responding
			processErrors(0);
			osal_usleep(mErrorCycleTime);
		}
	}


	void EtherCATMaster::processErrors(int slaveGroup)
	{
		ec_group[slaveGroup].docheckstate = false;
		ec_readstate();

		for (int slave = 1; slave <= ec_slavecount; slave++)
		{
			if ((ec_slave[slave].group == slaveGroup) && (ec_slave[slave].state != static_cast<uint16>(ESlaveState::Operational)))
			{
				// Signal slave error in group
				ec_group[slaveGroup].docheckstate = true;

				// In safe state with error bit set
				if (ec_slave[slave].state == static_cast<uint16>(EtherCATMaster::ESlaveState::SafeOperational) + 
					static_cast<uint16>(EtherCATMaster::ESlaveState::Error))
				{
					nap::Logger::error("%s: slave %d is in SAFE_OP + ERROR, attempting ack", this->mID.c_str(), slave);
					ec_slave[slave].state = (
						static_cast<uint16>(EtherCATMaster::ESlaveState::SafeOperational) + 
						static_cast<uint16>(EtherCATMaster::ESlaveState::ACK));
					writeState(slave);
				}

				// Safe operational
				else if (ec_slave[slave].state == static_cast<uint16>(EtherCATMaster::ESlaveState::SafeOperational))
				{
					nap::Logger::warn("%s: slave %d is in SAFE_OP, change to OPERATIONAL", this->mID.c_str(), slave);
					onSafeOperational(&(ec_slave[slave]), slave);
					ec_slave[slave].state = static_cast<uint16>(EtherCATMaster::ESlaveState::Operational);
					writeState(slave);
					
					// Call onoperational if state changed
					if (checkState(slave, ESlaveState::Operational) == ESlaveState::Operational)
					{
						onOperational(&(ec_slave[slave]), slave);
						nap::Logger::info("%s: slave %d OPERATIONAL", mID.c_str(), slave);
					}
					else
					{
						nap::Logger::info("%s: slave %d unable to reach OPERATIONAL state");
					}
				}
				
				// Slave in between none and safe operational
				else if (ec_slave[slave].state > static_cast<uint16>(EtherCATMaster::ESlaveState::None))
				{
					if (ec_reconfig_slave(slave, mRecoveryTimeout))
					{
						ec_slave[slave].islost = false;
						nap::Logger::info("%s: slave %d reconfigured", this->mID.c_str(), slave);
					}
				}
				
				// Slave might have gone missing 
				else if (!isLost(slave))
				{
					// Check if the slave is operational
					// If slave appears to be missing (none) tag it.
					checkState(slave, ESlaveState::Operational, EC_TIMEOUTSTATE / 1000);
					if (ec_slave[slave].state == static_cast<uint16>(ESlaveState::None))
					{
						ec_slave[slave].islost = true;
					}
				}
			}

			// Slave appears to be lost, 
			if (isLost(slave))
			{
				if (ec_slave[slave].state == static_cast<uint16>(ESlaveState::None))
				{
					if (ec_recover_slave(slave, mRecoveryTimeout))
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
		if (!ec_group[slaveGroup].docheckstate)
			nap::Logger::info("%s: all slaves resumed OPERATIONAL", this->mID.c_str());
	}


	EtherCATMaster::ESlaveState EtherCATMaster::requestState(ESlaveState state, int timeout)
	{
		// Force state for all slaves
		ec_slave[0].state = static_cast<uint16>(state);
		writeState(0);
		checkState(0, state, timeout);
		return static_cast<EtherCATMaster::ESlaveState>(ec_slave[0].state);
	}


	void EtherCATMaster::writeState(int index)
	{
		ec_writestate(index);
	}


	EtherCATMaster::ESlaveState EtherCATMaster::checkState(int index, ESlaveState state, int timeout)
	{
		return static_cast<EtherCATMaster::ESlaveState>(
			ec_statecheck(index, static_cast<uint16>(state), timeout * 1000));
	}
}