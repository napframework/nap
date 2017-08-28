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
		mNode = artnet_new(NULL, verbose);
		if (!errorState.check(mNode != nullptr, "Unable to create new art-net connection, error: %s", artnet_strerror()))
			return false;

		// Set node name and type (server)
		artnet_set_short_name(mNode, "artnet-nap");
		artnet_set_long_name(mNode, "Artnet NAP Node");
		
		// The node is an artnet server
		artnet_set_node_type(mNode, ARTNET_SRV);

		// set poll reply handler
		// artnet_set_handler(mNode, ARTNET_REPLY_HANDLER, reply_handler, NULL);

		// Start connection
		if (!errorState.check(artnet_start(mNode) == 0, "Unable to start art-net connection, error: %s", artnet_strerror()))
			return false;

		// broadcast a poll request
		if (!errorState.check(artnet_send_poll(mNode, NULL, ARTNET_TTM_DEFAULT) == ARTNET_EOK, "art-net send poll failed\n"))
			return false;

		return true;
	}
}