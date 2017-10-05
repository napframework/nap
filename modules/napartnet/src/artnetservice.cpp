// Local Includes
#include "artnetservice.h"
#include "artnetcontroller.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <artnet/artnet.h>
#include <iostream>

RTTI_DEFINE(nap::ArtNetService)

static bool verbose = true;

namespace nap
{
	// ctor delibarately in cpp for unique_ptr
	ArtNetService::ArtNetService()
	{
	}


	// dtor delibarately in cpp for unique_ptr
	ArtNetService::~ArtNetService()
	{
	}


	bool ArtNetService::addController(ArtNetController& controller, utility::ErrorState& errorState)
	{
		if (!errorState.check(mControllers.find(controller.getAddress()) == mControllers.end(), "Controller %s has the same address as a controller that has already been added"))
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
	}


	void ArtNetService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<ArtNetNodeCreator>(*this));
	}


	void ArtNetService::update()
	{
		double current_time = getCore().getElapsedTime();
		for (auto& controller : mControllers)
		{
			ControllerData* controller_data = controller.second.get();

			// Controllers are updated when:
			// - There's new data and the max frequency interval has passed
			// - There's no new data, but four seconds have passed. This is required for ArtNet. Resend existing data in that case.
			double time_since_last_update = current_time - controller_data->mLastUpdateTime;
			if ((controller_data->mIsDirty && time_since_last_update >= mUpdateFrequency) || time_since_last_update >= 4.0)
			{
				ArtNetNode node = controller.second->mController->getNode();

				int result = artnet_send_dmx(node, 0, controller_data->mData.size(), controller_data->mData.data());
				assert(result == ARTNET_EOK);

				controller_data->mIsDirty = false;
				controller_data->mLastUpdateTime = current_time;
			}
		}
	}
}