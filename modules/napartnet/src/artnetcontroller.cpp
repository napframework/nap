// Local Includes
#include "artnetcontroller.h"
#include "artnetservice.h" 

// External Includes
#include <artnet/artnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility/datetimeutils.h>
#include <thread>
#include <iostream>
#include <nap/logger.h>
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::EArtnetMode)
	RTTI_ENUM_VALUE(nap::EArtnetMode::Broadcast, "Broadcast"),
	RTTI_ENUM_VALUE(nap::EArtnetMode::Unicast,	 "Unicast")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::ArtNetController)
	RTTI_PROPERTY("Subnet",			&nap::ArtNetController::mSubnet,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Universe",		&nap::ArtNetController::mUniverse,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WaitTime",		&nap::ArtNetController::mWaitTime,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Frequency",		&nap::ArtNetController::mUpdateFrequency,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Mode",			&nap::ArtNetController::mMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UnicastLimit",	&nap::ArtNetController::mUnicastLimit,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Timeout",		&nap::ArtNetController::mTimeOut,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",		&nap::ArtNetController::mVerbose,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static const int mMaxUpdateFrequency = 44;

	ArtNetController::ArtNetController(ArtNetService& service) :
		mService(&service)
	{
	}

	ArtNetController::~ArtNetController()
	{
		stop();
	}
 

	bool ArtNetController::start(nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(mSubnet < 16, "%s: Subnet must be between 0 and 16", mID.c_str()))
			return false;

		if (!errorState.check(mUniverse < 16, "%s: Universe must be between 0 and 16", mID.c_str()))
			return false;

		// Create a new artnet (controller) node
		assert(mNode == nullptr);
		mNode = artnet_new(NULL, mVerbose ? 1 : 0);

		// Add controller
		if (!mService->addController(*this, errorState))
			return false;

		// Set node communication properties
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

		// get the socket descriptor
		mSocketDescriptor = artnet_get_sd(mNode);

		// Reset timeout for reading packages over network
		setTimeout(static_cast<uint32>(mTimeOut * 1000.0f));

		// Set the broadcast limit
		// When in unicast mode spin up an a-sync task to keep track of the
		// available nodes in the network. This ensures data is send to nodes based on the 
		// latest available network information
		switch (mMode)
		{
			case EArtnetMode::Broadcast:
			{
				artnet_set_bcast_limit(mNode, 0);
				break;
			}
			case EArtnetMode::Unicast:
			{
				artnet_set_bcast_limit(mNode, mUnicastLimit);
				mReadTask = std::async(std::launch::async, std::bind(&ArtNetController::pollAndRead, this));
				break;
			}
			default:
				assert(false);
		}

		return true;
	}


	void ArtNetController::stop()
	{
		if (mNode != nullptr)
		{
			if (mReadTask.valid())
			{
				stopPolling();
				mReadTask.wait();
			}

			mService->removeController(*this);
			artnet_destroy(mNode);
			mNode = nullptr;
		}
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


	void ArtNetController::setTimeout(uint32 milliseconds)
	{
		mActiveTime = milliseconds;
	}


	void ArtNetController::stopPolling()
	{
		setTimeout(0);
		mKeepPolling = false;
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


	const int ArtNetController::getMaxUpdateFrequency()
	{
		return mMaxUpdateFrequency;
	}


	void ArtNetController::pollAndRead()
	{
		nap::utility::SystemTimer ntimer;
		nap::utility::SystemTimer rtimer;

		// Acquire network nodes information while running
		while (mKeepPolling)
		{
			// Reset global timer
			ntimer.reset();

			// Poll other nodes on the network
			if (artnet_send_poll(mNode, NULL, ARTNET_TTM_DEFAULT) != 0)
			{
				nap::Logger::warn("Artnet poll request failed: %s", mID.c_str());
			}
			else
			{
				// Some select variables
				fd_set rset;
				struct timeval tv;

				// wait for timeout (x milliseconds) before timing out read request
				rtimer.reset();
				while (rtimer.getTicks() < mActiveTime)
				{
					// Setup file socket descriptors
					FD_ZERO(&rset);
					FD_SET(mSocketDescriptor, &rset);
					tv.tv_usec = mActiveTime - ((mActiveTime / 1000) * 1000);
					tv.tv_sec  = mActiveTime / 1000;

					// Wait for reply
					switch (select(mSocketDescriptor + 1, &rset, NULL, NULL, &tv))
					{
					case 0:
					{
						nap::Logger::warn("Artnet network poll request timed out: %s", mID.c_str());
						break;
					}	
					case -1:
					{
						nap::Logger::warn("Artnet network select error: %s", mID.c_str());
						break;
					}
					default:
					{
						if (artnet_read(mNode, 0) != 0)
						{
							nap::Logger::warn("Artnet read request failed: %s", mID.c_str());
						}
						break;
					}
					}
				}
			}

			// Don't poll as fast as the read speed over the network
			// Always wait at least to timeout setting
			std::chrono::milliseconds wait_time(mActiveTime);
			std::chrono::milliseconds comp_time(nap::math::min<uint32>(ntimer.getTicks(), mActiveTime));
			std::this_thread::sleep_for(wait_time - comp_time);
		}
	}
}