// Local Includes
#include "etherdreamservice.h"
 
// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>

namespace nap
{
	EtherDreamService::EtherDreamService()
	{
		// Create the interface
		mInterface = std::make_unique<EtherDreamInterface>();
	}


	EtherDreamService::~EtherDreamService()
	{
		mInterface->close();
	}


	bool EtherDreamService::init(nap::utility::ErrorState& errorState)
	{
		// Initialize
		if (!errorState.check(mInterface->init(), "Failed to initialize etherdream library"))
			return false;

		// Print some info
		nap::Logger::info("Initialized Etherdream library: found %d DAC(s)", mInterface->getCount());
		for (int i = 0; i < mInterface->getCount(); i++)
			nap::Logger::info("DAC name: %s", mInterface->getName(i).c_str());

		return true;
	}


	void EtherDreamService::registerObjectCreators(rtti::Factory& factory)
	{		
		factory.addObjectCreator(std::make_unique<DacObjectCreator>(*this));
	}


	int EtherDreamService::getIndex(const std::string& name)
	{
		for (int i = 0; i < mInterface->getCount(); i++)
		{
			if (name == mInterface->getName(i))
			{
				return i;
			}
		}
		return -1;
	}


	nap::EtherDreamInterface* EtherDreamService::getInterface() const
	{
		return mInterface.get();
	}


	bool EtherDreamService::addDAC(EtherDreamDac& dac)
	{
		// Get the associated number and set it
		int dac_number = getIndex(dac.mDacName);
		dac.mIndex = dac_number;

		// If the dac isn't available to the system we return false
		if (dac_number == -1)
		{
			nap::Logger::warn("DAC with name: %s is not available", dac.mDacName.c_str());
			return false;
		}

		// Register and return number
		mAvailableDacs.emplace(std::make_pair(dac_number, &dac));
		return true;
	}


	// Removes the dac from the system
	void EtherDreamService::removeDAC(EtherDreamDac& dac)
	{
		auto it = mAvailableDacs.find(dac.mIndex);
		if (it == mAvailableDacs.end())
			return;

		mAvailableDacs.erase(it);
	}
}

RTTI_DEFINE(nap::EtherDreamService)