// Local Includes
#include "artnetcontroller.h"
 
// External Includes
#include <artnet/artnet.h>
#include "artnetservice.h"

RTTI_BEGIN_CLASS(nap::ArtNetController)
	RTTI_PROPERTY("Subnet",		&nap::ArtNetController::mSubnet,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Universe",	&nap::ArtNetController::mUniverse,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WaitTime",	&nap::ArtNetController::mWaitTime,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Frequency",	&nap::ArtNetController::mUpdateFrequency,	nap::rtti::EPropertyMetaData::Default)
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
		
		// Set artnet port
		if (!errorState.check(artnet_set_port_addr(mNode, 0, ARTNET_INPUT_PORT, mUniverse) == 0, "Unable to set port address of ArtNode: %s error: %s", mID.c_str(), artnet_strerror()))
			return false;

		// Set artnet subnet address
		if (!errorState.check(artnet_set_subnet_addr(mNode, mSubnet) == 0, "Unable to set ArtNode subnet address: %s error: %s", mID.c_str(), artnet_strerror()))
			return false;

		// Start running
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


	void ArtNetController::send(float channelData, int channel)
	{
		mService->send(*this, channelData, channel);
	}


	void ArtNetController::send(const ByteChannelData& channelData, int channelOffset)
	{
		mService->send(*this, channelData, channelOffset);
	}


	void ArtNetController::send(uint8_t channelData, int channel)
	{
		mService->send(*this, channelData, channel);
	}


	void ArtNetController::clear()
	{
		mService->clear(*this);
	}


	nap::ArtNetController::Address ArtNetController::createAddress(uint8_t subnet, uint8_t universe)
	{
		return (subnet << 4) | universe;
	}


	void ArtNetController::convertAddress(Address address, uint8_t& subnet, uint8_t& universe)
	{
		subnet = address >> 4;
		universe = address & 0xF;
	}
}