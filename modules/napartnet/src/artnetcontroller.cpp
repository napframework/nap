/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetcontroller.h"
#include "artnetservice.h" 

// External Includes
#include <artnet/artnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <nap/timer.h>
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
	RTTI_PROPERTY("IP Address",		&nap::ArtNetController::mIpAddress,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WaitTime",		&nap::ArtNetController::mWaitTime,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Frequency",		&nap::ArtNetController::mUpdateFrequency,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Mode",			&nap::ArtNetController::mMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UnicastLimit",	&nap::ArtNetController::mUnicastLimit,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",		&nap::ArtNetController::mVerbose,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Timeout",		&nap::ArtNetController::mReadTimeout,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DataSize",		&nap::ArtNetController::mDataSize,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	ArtNetController::ArtNetController(ArtNetService& service) :
		mService(&service)
	{
	}
 

	bool ArtNetController::start(nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(mSubnet < 16, "%s: Subnet must be between 0 and 16", mID.c_str()))
			return false;

		if (!errorState.check(mUniverse < 16, "%s: Universe must be between 0 and 16", mID.c_str()))
			return false;

		if (!errorState.check(mDataSize >= 2 && mDataSize <= 512, "%s: DataSize must be between 2 and 512", mID.c_str()))
			return false;

		// Create a new artnet (controller) node
		assert(mNode == nullptr);
		mNode = artnet_new((mIpAddress.empty() ? NULL : mIpAddress.c_str()), (mVerbose ? 1 : 0));

		if (!errorState.check(mNode != nullptr,
			"Unable to create new ArtNode using address: %s error: %s", mIpAddress.c_str(), artnet_strerror()))
			return false;

		// Add controller
		if (!mService->addController(*this, errorState))
			return false;

		// Set node communication properties
		artnet_set_short_name(mNode, "artnet-nap");
		artnet_set_long_name(mNode, "Artnet NAP Node");
		artnet_set_node_type(mNode, ARTNET_SRV);
		artnet_set_port_type(mNode, 0, ARTNET_ENABLE_INPUT, ARTNET_PORT_DMX);

		// Set artnet port
		if (!errorState.check(artnet_set_port_addr(mNode, 0, ARTNET_INPUT_PORT, mUniverse) == 0,
			"Unable to set port address of ArtNode: %s error: %s", mID.c_str(), artnet_strerror()))
			return false;

		// Set artnet subnet address
		if (!errorState.check(artnet_set_subnet_addr(mNode, mSubnet) == 0,
			"Unable to set ArtNode subnet address: %s error: %s", mID.c_str(), artnet_strerror()))
			return false;

		// Start running
		if (!errorState.check(artnet_start(mNode) == 0,
			"Unable to start ArtNode node %s, error: %s", mID.c_str(), artnet_strerror()))
			return false;

		// get the socket descriptor
		mSocketDescriptor = artnet_get_sd(mNode);

		// Read timeout can't be more than 3 seconds, which is the poll rate
		// Artnet also specifies that nodes must respond within 3 seconds
		mReadTimeout = nap::math::min<float>(mReadTimeout, 3.0f);

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
		// Stop polling background task
		assert(mNode != nullptr);
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


	void ArtNetController::send(uint8 channelData, int channel)
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
		// Ensure we're not managing an already existing task
		assert(!mReadTask.valid());

		// Set the poll related variables thread safe
		mPoll = true;
		mExit = false;
		mPollTimer.start();

		// Fire up the reading task
		mReadTask = std::async(std::launch::async, std::bind(&ArtNetController::exePollTask, this));
	}


	void ArtNetController::poll()
	{
		if (mVerbose)
			nap::Logger::info("issuing new poll request: %s", mID.c_str());

		// Actual artnet poll request
		if (artnet_send_poll(mNode, NULL, ARTNET_TTM_DEFAULT) != 0)
		{
			if (mVerbose)
				nap::Logger::warn("artnet poll request failed: %s", mID.c_str());
			return;
		}

		// Initialize file descriptor and timer for select operation
		fd_set rset;
		struct timeval tv;
		int socket_d = mSocketDescriptor;

		// Gather network responses based on the last poll request
		// This gives artnet the chance to update it's node list internally
		nap::SystemTimer select_timer;
		select_timer.start();
		while (select_timer.getElapsedTime() < mReadTimeout)
		{
			// If exit was called break out of this loop
			if(mExit) 
				break;

			// Setup socket descriptor
			FD_ZERO(&rset);
			FD_SET(socket_d, &rset);
			tv.tv_usec = 0;
			tv.tv_sec  = 1;
			int maxsd = socket_d;
			
			// Wait for reply
			switch (select(maxsd + 1, &rset, NULL, NULL, &tv))
			{
				case 0:
				{
					if (mVerbose)
						nap::Logger::warn("artnet network poll request timed out: %s", mID.c_str());
					break;
				}
				case -1:
				{
					if (mVerbose)
						nap::Logger::warn("artnet network select error: %s", mID.c_str());
					break;
				}
				default:
				{
					// I assume here that reading is thread safe.
					// The documentation states that only manipulating nodes lists is not thread safe
					// Leading to the assumption that actually reading a reply is handled internally in a thread safe way
					// If this is not the case make sure to protect the read here to ensure the list is not whilst sending dmx data. 
					// Sending is handled inside the artnet service
					if (mVerbose)
						nap::Logger::info("reading node information: %s", mID.c_str());
					artnet_read(mNode, 0);
					break;
				}
			}
		}
	}


	nap::ArtNetController::Address ArtNetController::createAddress(uint8 subnet, uint8 universe)
	{
		return (subnet << 4) | universe;
	}


	void ArtNetController::convertAddress(Address address, uint8& subnet, uint8& universe)
	{
		subnet = address >> 4;
		universe = address & 0xF;
	}


	void ArtNetController::update(double deltaTime)
	{
		if (mMode == EArtnetMode::Broadcast)
			return;

		// Read data from socket if required, always do this before issuing a new poll request
		// Issue that request when time since previous poll exceeds timeout limit
		{
			// Check if we need to perform another poll request
			// If so notify the task by changing the value and waking it up
			if (mPollTimer.getElapsedTimeFloat() > 3.0f)
			{
				std::lock_guard<std::mutex> lock(mPollMutex);
				mPoll = true;
				mPollTimer.reset();
				mConditionVar.notify_one();
			}
		}
	}


	void ArtNetController::exePollTask()
	{
		// Acquire network nodes information while running
		while (true)
		{
			// Wait for a poll request
			if (mVerbose)
				nap::Logger::info("waiting for artnet poll request");
			
			// The conditional variable makes sure the poll only executes
			// when the main thread tiggers a poll request
			// If the poll is slower than the main thread, ie:
			// the main thread issues a poll request before the previous request finishes,
			// this task waits for the next poll request to come in.
			{
				std::unique_lock<std::mutex> lock(mPollMutex);
				mConditionVar.wait(lock, [this]()
				{
					return (mPoll || mExit);
				});

				// Exit poll loop when exit has been triggered
				if (mExit)
				{
					break;
				}
			}

			// Poll, afterwards set the poll flag to false, forcing a wait until a new poll request
			poll();
			std::lock_guard<std::mutex> lock(mPollMutex);
			mPoll = false;
		}
		nap::Logger::info("ArtNet poll task ended: %s", mID.c_str());
	}
}
