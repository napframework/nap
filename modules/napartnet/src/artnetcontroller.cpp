// Local Includes
#include "artnetcontroller.h"
 
// External Includes
#include <artnet/artnet.h>
#include "artnetservice.h"

RTTI_BEGIN_CLASS(nap::ArtNetController)
	RTTI_PROPERTY("Subnet",		&nap::ArtNetController::mSubnet,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Universe",	&nap::ArtNetController::mUniverse,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	ArtNetController::ArtNetController(ArtNetService& service) :
		mService(&service)
	{
	}

	ArtNetController::~ArtNetController()
	{
		if (mNode != nullptr)
		{
			mService->removeController(*this);
			artnet_destroy(mNode);			
		}
	}

	bool ArtNetController::init(nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(mSubnet < 16, "%s: Subnet must be between 0 and 16", mID.c_str()))
			return false;

		if (!errorState.check(mUniverse < 16, "%s: Universe must be between 0 and 16", mID.c_str()))
			return false;

		mNode = artnet_new(NULL, 0);

		artnet_set_short_name(mNode, "artnet-nap");
		artnet_set_long_name(mNode, "Artnet NAP Node");
		artnet_set_node_type(mNode, ARTNET_SRV);
		artnet_set_port_type(mNode, 0, ARTNET_ENABLE_INPUT, ARTNET_PORT_DMX);		
		artnet_set_port_addr(mNode, 0, ARTNET_INPUT_PORT, getAddress());

		if (!errorState.check(artnet_start(mNode) == 0, "Unable to start ArtNode node %s, error: %s", mID.c_str(), artnet_strerror()))
			return false;
		
		if (!mService->addController(*this, errorState))
			return false;

		return true;
	}

	void ArtNetController::send(const FloatChannelData& channelData, int channelOffset)
	{
		mService->send(*this, channelData, channelOffset);
	}

	void ArtNetController::send(const ByteChannelData& channelData, int channelOffset)
	{
		mService->send(*this, channelData, channelOffset);
	}
}