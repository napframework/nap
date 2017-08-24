// Local Includes
#include "artnetservice.h"
 
// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <artnet/artnet.h>

RTTI_DEFINE(nap::ArtnetService)

static bool verbose = true;

namespace nap
{
	ArtnetService::~ArtnetService()
	{
		if (mNode != nullptr)
		{
			if (artnet_stop(mNode) != 0)
			{
				nap::Logger::warn("unable to stop Artnet service");
			}
			mNode = nullptr;
		}
	}


	bool ArtnetService::init(nap::utility::ErrorState& errorState)
	{
		// Create new artnet instance
		mNode = artnet_new(mIpAddress.c_str(), verbose);
		if (!errorState.check(mNode != nullptr, "Unable to create new Artnet connection, ip: %s", mIpAddress.c_str()))
			return false;
		
		// Start connection
		if (!errorState.check(artnet_start(mNode) == 0, "Unable to start Artnet connection, ip: %s", mIpAddress.c_str()))
			return false;

		return true;
	}

}