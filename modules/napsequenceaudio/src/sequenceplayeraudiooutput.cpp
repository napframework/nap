/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudiooutput.h"

#include <audio/service/audioservice.h>
#include <sequenceservice.h>
#include <nap/core.h>
#include <audio/resource/audiobufferresource.h>
#include <nap/logger.h>

RTTI_BEGIN_ENUM(nap::SequencePlayerAudioOutput::ESequencePlayerAudioOutputMode)
    RTTI_ENUM_VALUE(nap::SequencePlayerAudioOutput::ESequencePlayerAudioOutputMode::DIRECT,		"Direct"),
    RTTI_ENUM_VALUE(nap::SequencePlayerAudioOutput::ESequencePlayerAudioOutputMode::MANUAL,		"Manual")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioOutput)
        RTTI_PROPERTY("OutputMode", &nap::SequencePlayerAudioOutput::mOutputMode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using namespace nap::audio;

namespace nap
{
	SequencePlayerAudioOutput::SequencePlayerAudioOutput(SequenceService& service)
		: SequencePlayerOutput(service){}


	bool SequencePlayerAudioOutput::init(utility::ErrorState& errorState)
	{
		auto* resource_manager = mService->getCore().getResourceManager();

		mPostResourcesLoadedSlot 	= Slot<>([this](){ onPostResourcesLoaded(); });
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
		for(auto& buffer_player : it->second)
		{
			buffer_player.second->play(discrete_time, playbackSpeed);
		}
	}


	void SequencePlayerAudioOutput::handleAudioSegmentStop(const SequencePlayerAudioAdapter* adapter, const std::string& id)
	{
		auto it = mBufferPlayers.find(adapter);
		assert(it!=mBufferPlayers.end()); // entry not found
		assert(it->second.find(id)!=it->second.end()); // entry not found
		for(auto& buffer_player : it->second)
		{
			buffer_player.second->stop();
		}
	}


	void SequencePlayerAudioOutput::registerAdapter(const SequencePlayerAudioAdapter* adapter)
	{
		assert(mBufferPlayers.find(adapter)==mBufferPlayers.end()); // adapter already registered

        /**
         * Get audio service and acquire audio node manager
         */
        mAudioService = mService->getCore().getService<AudioService>();
        auto& node_manager = mAudioService->getNodeManager();

        /**
         * Create buffer player for each available buffer
         */
        std::unordered_map<std::string, SafeOwner<MultiSampleBufferPlayerNode>> buffer_players;
        for (auto buffer : mAudioBuffers)
        {
            int channel_count = buffer->getChannelCount();

            // create buffer player
            auto buffer_player = node_manager.makeSafe<MultiSampleBufferPlayerNode>(channel_count, node_manager);
            buffer_player->setBuffer(buffer->getBuffer());

            for(int i = 0; i < channel_count; i++)
            {
                mMixNodes[i]->inputs.enqueueConnect(*buffer_player->getOutputPins()[i]);
            }

            // create new entry  of buffer player
            buffer_players.emplace(buffer->mID, std::move(buffer_player));
        }

        // move created map of buffer players and output nodes
        mBufferPlayers.emplace(adapter, std::move(buffer_players));
	}


	void SequencePlayerAudioOutput::unregisterAdapter(const SequencePlayerAudioAdapter* adapter)
	{
		assert(mBufferPlayers.find(adapter)!=mBufferPlayers.end()); // entry not found

        mAudioService = mService->getCore().getService<AudioService>();
        auto& node_manager = mAudioService->getNodeManager();

        for(auto& buffer_player_entry : mBufferPlayers)
        {
            for(auto& buffer_player : buffer_player_entry.second)
            {
                auto output_pins = buffer_player.second->getOutputPins();
                for(int i = 0 ; i < output_pins.size(); i++)
                {
                    assert(i < mMixNodes.size());
                    mMixNodes[i]->inputs.enqueueDisconnect(*output_pins[i]);
                }
            }
        }

        mBufferPlayers.erase(adapter);
	}


	void SequencePlayerAudioOutput::onDestroy()
	{
		mBufferPlayers.clear();
		mOutputNodes.clear();

        auto* resource_manager = mService->getCore().getResourceManager();
        resource_manager->mPostResourcesLoadedSignal.disconnect(mPostResourcesLoadedSlot);
	}


	void SequencePlayerAudioOutput::update(double deltaTime)
	{
	}


	void SequencePlayerAudioOutput::onPostResourcesLoaded()
	{
        /**
         * Acquire all loaded audio buffers
         */
		auto* resource_manager = mService->getCore().getResourceManager();
		auto audio_buffers = resource_manager->getObjects<AudioBufferResource>();
		nap::Logger::info(*this, "found %i audio buffers", audio_buffers.size());
		mAudioBuffers = audio_buffers;

        /**
         * Get audio service and acquire audio node manager
         */
        mAudioService = mService->getCore().getService<AudioService>();
        auto& node_manager = mAudioService->getNodeManager();

        /**
         * Get maximum amount of channels that can be used by available audio buffers and make that amount of mix nodes and
         * when output mode is direct also make output nodes that connect with mix nodes
         */
        int max_channels = 0;
        for (auto buffer : mAudioBuffers)
        {
            if(buffer->getChannelCount() > max_channels)
                max_channels = buffer->getChannelCount();
        }
        // create mix nodes
        std::vector<SafeOwner<MixNode>> mix_nodes;
        for(int i = 0 ; i < max_channels; i++)
        {
            auto mix_node = node_manager.makeSafe<MixNode>(node_manager);
            mix_nodes.emplace_back(std::move(mix_node));
        }
        std::vector<SafeOwner<OutputNode>> output_nodes;
        if(mOutputMode==ESequencePlayerAudioOutputMode::DIRECT)
        {
            for(int i = 0 ; i < max_channels; i++)
            {
                auto output_node = node_manager.makeSafe<OutputNode>(node_manager);
                output_node->setOutputChannel(i);
                output_node->audioInput.connect(mix_nodes[i]->audioOutput);
                output_nodes.emplace_back(std::move(output_node));
            }
        }
        mMixNodes = std::move(mix_nodes);
        mOutputNodes = std::move(output_nodes);
	}


	const std::vector<rtti::ObjectPtr<audio::AudioBufferResource>>& SequencePlayerAudioOutput::getBuffers() const
	{
		return mAudioBuffers;
	}
}