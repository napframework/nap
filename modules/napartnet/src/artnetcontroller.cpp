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
				startPolling();
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
			// Stop polling background task
			if (mMode == EArtnetMode::Unicast)
			{
				assert(mReadTask.valid());
				stopPolling();
				mReadTask.wait();
			}

			// Remove controller from service
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


	void ArtNetController::stopPolling()
	{
		std::lock_guard<std::mutex> lock(mPollMutex);
		mExit = true;
		mConditionVar.notify_one();
	}


	void ArtNetController::startPolling()
	{
		// Set the poll related variables thread safe
		std::lock_guard<std::mutex> lock(mPollMutex);
		mPoll = true;
		mRead = false;
		mExit = false;
		mPollTimer.start();

		// Fire up the reading task
		mReadTask = std::async(std::launch::async, std::bind(&ArtNetController::pollAndRead, this));
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


	void ArtNetController::update(double deltaTime)
	{
		if (mMode == EArtnetMode::Broadcast)
			return;

		// Read data from socket if required, always do this before issuing a new poll request
		// Issue that request when time since previous poll exceeds timeout limit
		{
			if (mRead)
			{
				if (mVerbose)
					nap::Logger::info("reading node information: %s", mID.c_str());
				std::lock_guard<std::mutex> lock(mPollMutex);
				artnet_read(mNode, 0);
				mRead = false;
			}

			// Check if we need to perform another poll request
			// If so notify the task by changing the value and waking it up
			if (mPollTimer.getElapsedTimeFloat() > mTimeOut)
			{
				std::lock_guard<std::mutex> lock(mPollMutex);
				mPoll = true;
				mPollTimer.reset();
				mConditionVar.notify_one();
			}
		}
	}


	void ArtNetController::pollAndRead()
	{
		// Acquire network nodes information while running
		while (true)
		{
			// Wait for a poll request, also, don't poll when still having to read
			// When the device is stopped make sure the task is exited asap 
			{
				std::unique_lock<std::mutex> lock(mPollMutex);
				mConditionVar.wait(lock, [this]()
				{
					return ((mPoll && !mRead) || mExit);
				});

				// Exit poll loop when exit has been triggered
				if (mExit)
				{
					break;
				}
			}

			// Perform a new poll
			{
				if (mVerbose)
					nap::Logger::info("issuing new poll request: %s", mID.c_str());

				// Poll other nodes on the network
				if (artnet_send_poll(mNode, NULL, ARTNET_TTM_DEFAULT) == 0)
				{
					// Some select variables
					fd_set rset;
					struct timeval tv;

					// wait for timeout (x milliseconds) before timing out read request
					// Setup file socket descriptors
					FD_ZERO(&rset);
					FD_SET(mSocketDescriptor, &rset);
					tv.tv_usec = int((mTimeOut - float(int(mTimeOut))) * 1000.0f);
					tv.tv_sec  = static_cast<int>(mTimeOut);

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
						// Set the read flag safely, mutex is released when exiting scope
						std::lock_guard<std::mutex> lock(mPollMutex);
						mRead = true;
						break;
					}
					}
				}
				else
				{
					nap::Logger::warn("Artnet poll request failed: %s", mID.c_str());
				}

				// Set the poll flag to false, forcing next iteration of poll request to wait until 
				// the main thread signals a new poll request
				std::lock_guard<std::mutex> lock(mPollMutex);
				mPoll = false;
			}
		}

		if(mVerbose)
			nap::Logger::info("ended artnet poll task: %s", mID.c_str());
	}
}