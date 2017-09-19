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
	ArtNetService::ArtNetService()
	{
	}

	ArtNetService::~ArtNetService()
	{
	}

	bool ArtNetService::addController(ArtNetController& node, utility::ErrorState& errorState)
	{
		if (!errorState.check(mControllers.find(node.getAddress()) == mControllers.end(), "Node %s has the same address as a node that has already been added"))
			return false;

		std::unique_ptr<ControllerData> controller_data = std::make_unique<ControllerData>();
		controller_data->mController = &node;
		controller_data->mData.resize(512);
		controller_data->mIsDirty = true;
		controller_data->mLastUpdateTime = 0.0f;

		mControllers.emplace(std::make_pair(node.getAddress(), std::move(controller_data)));
		
		return true;
	}

	void ArtNetService::removeController(ArtNetController& node)
	{
		mControllers.erase(node.getAddress());
	}

	void ArtNetService::send(ArtNetController& inNode, const FloatChannelData& channelData, int channelOffset)
	{
		ByteChannelData data;
		data.resize(channelData.size());

		for (int index = 0; index < channelData.size(); ++index)
		{
			assert(channelData[index] >= 0.0f && channelData[index] <= 1.0f);
			data[index] = (uint8_t)(channelData[index] * 255.0f);
		}

		send(inNode, data, channelOffset);
	}

	void ArtNetService::send(ArtNetController& inNode, const ByteChannelData& channelData, int channelOffset)
	{
		NodeMap::iterator pos = mControllers.find(inNode.getAddress());
		assert(pos != mControllers.end());
		
		ByteChannelData& channel_data = pos->second->mData;
		assert(channelOffset + channelData.size() <= channel_data.size());
		
		std::memcpy(channel_data.data() + channelOffset, channelData.data(), channelData.size());
		pos->second->mIsDirty = true;
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