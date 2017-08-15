#include "etherdreamdac.h"
#include "etherdreamservice.h"
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::EtherDreamDac)
	RTTI_PROPERTY("DacName",	&nap::EtherDreamDac::mDacName,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointRate",	&nap::EtherDreamDac::mPointRate,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	EtherDreamDac::EtherDreamDac(EtherDreamService& service) : mService(&service)
	{}

	EtherDreamDac::~EtherDreamDac()
	{
		// Disconnect the dac
		if (isConnected())
		{
			nap::Logger::info("Disconnecting Etherdream DAC: %s, index: %d", mDacName.c_str(), mIndex);
			mService->getInterface()->disconnect(mIndex);
		}

		// Remove dac from service
		mService->removeDAC(*this);
	}


	bool EtherDreamDac::init(utility::ErrorState& errorState)
	{
		assert(mService != nullptr);
		
		// Add the DAC to the system
		// If the call returns true the DAC is valid and has been added, otherwise the DAC isn't
		// available to the current systems
		if (!mService->addDAC(*this))
		{
			nap::Logger::warn("Unable to find Etherdream DAC with name: %s", mDacName.c_str());
			mStatus = EConnectionStatus::UNAVAILABLE;
			return true;
		}

		// Connect the dac
		if (!mService->getInterface()->connect(mIndex))
		{
			nap::Logger::warn("Unable to connect to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
			mStatus = EConnectionStatus::CONNECTION_ERROR;
			return true;
		}
		else
		{
			nap::Logger::info("Successfully connected to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
		}

		mStatus = EConnectionStatus::CONNECTED;
		return true;
	}


	nap::EtherDreamInterface::EStatus EtherDreamDac::getWriteStatus() const
	{
		return isConnected() ? mService->getInterface()->getStatus(mIndex) : EtherDreamInterface::EStatus::ERROR;
	}
	

	bool EtherDreamDac::isReady() const
	{
		return getWriteStatus() == EtherDreamInterface::EStatus::READY;
	}


	bool EtherDreamDac::writeFrame(EtherDreamPoint* data, uint npoints)
	{
		return mService->getInterface()->writeFrame(mIndex, data, npoints, static_cast<uint>(mPointRate), 1);
	}

}