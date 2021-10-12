/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudiooutput.h"

#include <audio/service/audioservice.h>
#include <sequenceservice.h>
#include <nap/core.h>
#include <audio/resource/audiobufferresource.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioOutput)
RTTI_END_CLASS

using namespace nap::audio;

namespace nap
{
	SequencePlayerAudioOutput::SequencePlayerAudioOutput(SequenceService& service)
		: SequencePlayerOutput(service){}


	bool SequencePlayerAudioOutput::init(utility::ErrorState& errorState)
	{
		auto* resource_manager = mService->getCore().getResourceManager();

		mPreResourcesLoadedSlot		= Slot<>([this](){onPreResourcesLoaded();});
		mPostResourcesLoadedSlot 	= Slot<>([this](){onPostResourcesLoaded();});
		resource_manager->mPostResourcesLoadedSignal.connect(mPostResourcesLoadedSlot);

		return true;
	}


	void SequencePlayerAudioOutput::handleAudioSegmentPlay(const SequencePlayerAudioAdapter* adapter,
														   const std::string& id,
														   double time,
														   float playbackSpeed)
	{
		auto& node_manager = mAudioService->getNodeManager();

		int64 discrete_time = node_manager.getSampleRate() * time;
		auto it = mBufferPlayers.find(adapter);
		assert(it!=mBufferPlayers.end()); // entry not found
		assert(it->second.find(id)!=it->second.end()); // entry not found
		for(auto& buffer_player : it->second[id])
		{
			buffer_player->play(0, discrete_time, playbackSpeed);
		}
	}


	void SequencePlayerAudioOutput::handleAudioSegmentStop(const SequencePlayerAudioAdapter* adapter, const std::string& id)
	{
		auto it = mBufferPlayers.find(adapter);
		assert(it!=mBufferPlayers.end()); // entry not found
		assert(it->second.find(id)!=it->second.end()); // entry not found
		for(auto& buffer_player : it->second[id])
		{
			buffer_player->stop();
		}
	}


	void SequencePlayerAudioOutput::registerAdapter(const SequencePlayerAudioAdapter* adapter)
	{
		assert(mBufferPlayers.find(adapter)==mBufferPlayers.end()); // adapter already registered
		assert(mOutputNodes.find(adapter)==mOutputNodes.end()); // adapter already registered

		std::unordered_map<std::string, std::vector<SafeOwner<BufferPlayerNode>>> buffer_players;
		std::vector<SafeOwner<OutputNode>> output_nodes;
		mAudioService = mService->getCore().getService<AudioService>();
		auto& node_manager = mAudioService->getNodeManager();
		for (auto buffer : mAudioBuffers)
		{
			for(int channel = 0; channel < buffer->getChannelCount(); channel++)
			{
				// create buffer player
				auto buffer_player = node_manager.makeSafe<BufferPlayerNode>(node_manager);
				buffer_player->setBuffer(buffer->getBuffer());

				// create output node
				auto output_node = node_manager.makeSafe<OutputNode>(node_manager);
				output_node->setOutputChannel(channel);
				output_node->audioInput.connect(buffer_player->audioOutput);

				// create new entry of vector of buffer players if necessary
				if(buffer_players.find(buffer->mID)==buffer_players.end())
				{
					buffer_players.emplace(buffer->mID, std::vector<SafeOwner<BufferPlayerNode>>());
				}

				// move the buffer player & output node
				buffer_players[buffer->mID].emplace_back(std::move(buffer_player));
				output_nodes.emplace_back(std::move(output_node));
			}
		}

		// move created map of buffer players and output nodes
		mOutputNodes.emplace(adapter, std::move(output_nodes));
		mBufferPlayers.emplace(adapter, std::move(buffer_players));
	}


	void SequencePlayerAudioOutput::unregisterAdapter(const SequencePlayerAudioAdapter* adapter)
	{
		assert(mBufferPlayers.find(adapter)!=mBufferPlayers.end()); // entry not found
		assert(mOutputNodes.find(adapter)!=mOutputNodes.end()); // entry not found

		mOutputNodes.erase(adapter);
		mBufferPlayers.erase(adapter);
	}


	void SequencePlayerAudioOutput::onDestroy()
	{
		mBufferPlayers.clear();
		mOutputNodes.clear();
	}


	void SequencePlayerAudioOutput::update(double deltaTime)
	{
	}


	void SequencePlayerAudioOutput::onPostResourcesLoaded()
	{
		auto* resource_manager = mService->getCore().getResourceManager();
		auto audio_buffers = resource_manager->getObjects<AudioBufferResource>();
		nap::Logger::info(*this, "found %i audio buffers", audio_buffers.size());
		mAudioBuffers = audio_buffers;

		resource_manager->mPreResourcesLoadedSignal.connect(mPreResourcesLoadedSlot);
	}


	void SequencePlayerAudioOutput::onPreResourcesLoaded()
	{
		auto* resource_manager = mService->getCore().getResourceManager();
		resource_manager->mPostResourcesLoadedSignal.disconnect(mPostResourcesLoadedSlot);
	}


	const std::vector<rtti::ObjectPtr<audio::AudioBufferResource>>& SequencePlayerAudioOutput::getBuffers() const
	{
		return mAudioBuffers;
	}
}