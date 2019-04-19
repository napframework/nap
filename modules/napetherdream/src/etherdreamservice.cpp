// Local Includes
#include "etherdreamservice.h"
 
// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EtherDreamService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	EtherDreamService::EtherDreamService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
		// Create the interface
		mInterface = std::make_unique<EtherDreamInterface>();
	}


	bool EtherDreamService::init(nap::utility::ErrorState& errorState)
	{
		// Initialize
		if (!errorState.check(mInterface->init(), "Failed to initialize etherdream library"))
			return false;

		// Print some info
		nap::Logger::info("Initialized Etherdream library: found %d DAC(s)", mInterface->getCount());
		for (int i = 0; i < mInterface->getCount(); i++)
		{
			std::string dac_name = mInterface->getName(i);
			mDacNames.emplace(std::make_pair(i, dac_name));
			nap::Logger::info("DAC name: %s", dac_name.c_str());
		}

		return true;
	}


	void EtherDreamService::shutdown()
	{
		mInterface->close();
	}


	void EtherDreamService::registerObjectCreators(rtti::Factory& factory)
	{		
		factory.addObjectCreator(std::make_unique<DacObjectCreator>(*this));
	}


	void EtherDreamService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
	}


	int EtherDreamService::getIndex(const std::string& name)
	{
		for (const auto& it : mDacNames)
		{
			if (it.second == name)
			{
				return it.first;
			}
		}
		return -1;
	}


	nap::EtherDreamInterface* EtherDreamService::getInterface() const
	{
		return mInterface.get();
	}


	bool EtherDreamService::allocateDAC(EtherDreamDac& dac)
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
		return true;
	}
}
