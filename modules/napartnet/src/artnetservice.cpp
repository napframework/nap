/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetservice.h"
#include "artnetcontroller.h"
#include "artnetreceiver.h"
#include "artnetinputcomponent.h"
#include "artnetevent.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <artnet/artnet.h>
#include <iostream>
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtNetService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	// ctor delibarately in cpp for unique_ptr
	ArtNetService::ArtNetService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	// dtor delibarately in cpp for unique_ptr
	ArtNetService::~ArtNetService()
	{
	}


	bool ArtNetService::addController(ArtNetController& controller, utility::ErrorState& errorState)
	{
		if (!errorState.check(mControllers.find(controller.getAddress()) == mControllers.end(),
                              "Controller %s has the same address as a controller that has already been added", controller.mID.c_str()))
			return false;

		const int numChannelsInUniverse = 512;

		std::unique_ptr<ControllerData> controller_data = std::make_unique<ControllerData>();
		controller_data->mController = &controller;
		controller_data->mData.resize(numChannelsInUniverse);
		controller_data->mIsDirty = true;
		controller_data->mLastUpdateTime = 0.0f;

		mControllers.emplace(std::make_pair(controller.getAddress(), std::move(controller_data)));

		return true;
	}


	void ArtNetService::removeController(ArtNetController& controller)
	{
		mControllers.erase(controller.getAddress());
	}


	bool ArtNetService::addReceiver(ArtNetReceiver& receiver, utility::ErrorState& errorState)
	{
		if (!errorState.check(mReceivers.find(receiver.mPort) == mReceivers.end(),
			"Receiver %s has the same port as a receiver that has already been added", receiver.mID.c_str()))
			return false;

		mReceivers.emplace(std::make_pair(receiver.mPort, &receiver));
		return true;
	}


	void ArtNetService::removeReceiver(ArtNetReceiver& receiver)
	{
		mReceivers.erase(receiver.mPort);
	}


	void ArtNetService::addInputComponent(ArtNetInputComponentInstance& input)
	{
		mInputs.emplace_back(&input);
	}


	void ArtNetService::removeInputComponent(ArtNetInputComponentInstance& input)
	{
		auto found_it = std::find_if(mInputs.begin(), mInputs.end(), [&](const auto& it)
			{
				return it == &input;
			});
		assert(found_it != mInputs.end());
		mInputs.erase(found_it);
	}


	void ArtNetService::send(ArtNetController& controller, const FloatChannelData& channelData, int channelOffset)
	{
		ByteChannelData data;
		data.resize(channelData.size());

		// Convert normalized float data to bytes
		for (int index = 0; index < channelData.size(); ++index)
		{
			assert(channelData[index] >= 0.0f && channelData[index] <= 1.0f);
			data[index] = (uint8_t)(channelData[index] * 255.0f);
		}

		send(controller, data, channelOffset);
	}


	void ArtNetService::send(ArtNetController& controller, float channelData, int channel)
	{
		assert(channelData >= 0.0f && channelData <= 1.0f);
		uint8_t byte_data = (uint8_t)(channelData * 255.0f);
		send(controller, byte_data, channel);
	}


	void ArtNetService::send(ArtNetController& controller, const ByteChannelData& channelData, int channelOffset)
	{
		ControllerMap::iterator pos = mControllers.find(controller.getAddress());
		assert(pos != mControllers.end());

		ByteChannelData& channel_data = pos->second->mData;
		assert(channelOffset + channelData.size() <= channel_data.size());

		// Copy into internal buffer that is sent on update
		std::memcpy(channel_data.data() + channelOffset, channelData.data(), channelData.size());
		pos->second->mIsDirty = true;
	}


	void ArtNetService::send(ArtNetController& controller, uint8_t channelData, int channel)
	{
		ControllerMap::iterator pos = mControllers.find(controller.getAddress());
		assert(pos != mControllers.end());
		assert(channel >= 0 && channel < pos->second->mData.size());

		pos->second->mData[channel] = channelData;
        pos->second->mIsDirty = true;
	}


	void ArtNetService::clear(ArtNetController& controller)
	{
		ControllerMap::iterator pos = mControllers.find(controller.getAddress());
		assert(pos != mControllers.end());
		std::fill(pos->second->mData.begin(), pos->second->mData.end(), 0);
		pos->second->mIsDirty = true;
	}


	void ArtNetService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<ArtNetNodeCreator>(*this));
		factory.addObjectCreator(std::make_unique<ArtNetReceiverCreator>(*this));
	}


	void ArtNetService::update(double deltaTime)
	{
		/*
		 * Update Art-Net Controllers
		 */

		double current_time = getCore().getElapsedTime();
		for (auto& controller : mControllers)
		{
			ControllerData* controller_data = controller.second.get();

			// Controllers are updated when:
			// - There's new data and the max frequency interval has passed
			// - There's no new data, but four seconds have passed. This is required for ArtNet. Resend existing data in that case.
			double time_since_last_update = current_time - controller_data->mLastUpdateTime;

			// Get the amount of seconds to use for auto refresh
			float controller_wait_time = controller_data->mController->mWaitTime;

			// Calculate update frequency
			double update_freq = 1.0 / static_cast<double>(math::clamp<int>(controller_data->mController->mUpdateFrequency, 1, 44));

			// Update controller node
			controller.second->mController->update(deltaTime);

			// Check if we need to send dmx data
			bool refresh = controller_data->mIsDirty && time_since_last_update >= update_freq;
			bool timeout = time_since_last_update > controller_wait_time;
			if (refresh || timeout)
			{
				// Get artnet node
				ArtNetNode node = controller.second->mController->getNode();

				// Send dmx info
				int result = artnet_send_dmx(node, 0, controller_data->mData.size(), controller_data->mData.data());
				assert(result == ARTNET_EOK);

				controller_data->mIsDirty = false;
				controller_data->mLastUpdateTime = current_time;
			}
		}

		/*
		 * Consume events from Art-Net Receivers
		 */

		std::queue<ArtDmxPacketEventPtr> events;

		// Forward every event to every input component of interest
		for (auto& receiverPair : mReceivers)
		{
			ArtNetReceiver* receiver = receiverPair.second;

			receiver->consumeEvents(events);

			// Keep forwarding events until the queue runs out
			while (!(events.empty()))
			{
				ArtNetEvent& event = *(events.front());
				for (const auto& input : mInputs)
				{
					// Always forward event when Receive All is true
					// or when the Net, SubNet and Universe match
					if (input->mReceiveAll || (
						input->mNet == event.getNet() &&
						input->mSubNet == event.getSubNet() &&
						input->mUniverse == event.getUniverse()))
						input->trigger(event);
				}
				events.pop();
			}
		}
	}
}
