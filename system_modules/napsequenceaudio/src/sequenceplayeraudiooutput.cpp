/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudiooutput.h"

#include <audio/service/audioservice.h>
#include <sequenceservice.h>
#include <nap/core.h>
#include <audio/resource/audiobufferresource.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioOutput, "Outputs the sound from the sequenced audio track")
    RTTI_PROPERTY("Audio Buffers", &nap::SequencePlayerAudioOutput::mAudioBuffers, nap::rtti::EPropertyMetaData::Default, "Audio output buffers")
    RTTI_PROPERTY("Manual Routing", &nap::SequencePlayerAudioOutput::mManualRouting, nap::rtti::EPropertyMetaData::Default, "Directly output to selected audio output device")
    RTTI_PROPERTY("Max Channels", &nap::SequencePlayerAudioOutput::mMaxChannels, nap::rtti::EPropertyMetaData::Default, "Maximum number of audio channels to use")
RTTI_END_CLASS

using namespace nap::audio;

namespace nap
{
    SequencePlayerAudioOutput::SequencePlayerAudioOutput(SequenceService& service)
            :SequencePlayerOutput(service)
    {
    }


    bool SequencePlayerAudioOutput::init(utility::ErrorState& errorState)
    {
        /**
         * Get audio service and acquire audio node manager and create a mix node for each channel.
         * All buffer players created when registering an adapter to this output will connect their audio output to the created
         * mix nodes for each channel.
         * When mManualRouting is false, the SequencePlayerAudioOutput will also create output nodes for each channel
         * so audio played by SequencePlayer gets routed to the playback device selected by the AudioService.
         */
        mAudioService = mService->getCore().getService<AudioService>();
        auto& node_manager = mAudioService->getNodeManager();

        // create mix nodes
        std::vector<SafeOwner<MixNode>> mix_nodes;
        for (int i = 0; i<mMaxChannels; i++)
        {
            auto mix_node = node_manager.makeSafe<MixNode>(node_manager);
            mix_nodes.emplace_back(std::move(mix_node));
        }

        // create output nodes
        std::vector<SafeOwner<OutputNode>> output_nodes;
        if (!mManualRouting)
        {
            for (int i = 0; i<mMaxChannels; i++)
            {
                auto output_node = node_manager.makeSafe<OutputNode>(node_manager);
                output_node->setOutputChannel(i);
                output_node->audioInput.connect(mix_nodes[i]->audioOutput);
                output_nodes.emplace_back(std::move(output_node));
            }
        }

        // move ownership
        mMixNodes = std::move(mix_nodes);
        mOutputNodes = std::move(output_nodes);

        return true;
    }


    void SequencePlayerAudioOutput::handleAudioSegmentPlay(const SequencePlayerAudioAdapter* adapter,
                                                           const std::string& id, double time, float playbackSpeed)
    {
        auto& node_manager = mAudioService->getNodeManager();

        int64 discrete_time = node_manager.getSampleRate()*time;
        auto it = mBufferPlayers.find(adapter);
        assert(it!=mBufferPlayers.end()); // entry not found
        assert(it->second.find(id)!=it->second.end()); // entry not found
        auto buffer_player_it = it->second.find(id);
        buffer_player_it->second->play(discrete_time, playbackSpeed);
    }


    void SequencePlayerAudioOutput::handleAudioSegmentStop(const SequencePlayerAudioAdapter* adapter,
                                                           const std::string& id)
    {
        auto it = mBufferPlayers.find(adapter);
        assert(it!=mBufferPlayers.end()); // entry not found
        assert(it->second.find(id)!=it->second.end()); // entry not found
        auto buffer_player_it = it->second.find(id);
        buffer_player_it->second->stop();
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

            for (int i = 0; i<channel_count; i++)
            {
                mMixNodes[i]->inputs.connect(*buffer_player->getOutputPins()[i]);
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

        /**
         * unregister adapter and destroy all associated buffer players
         */
        mAudioService = mService->getCore().getService<AudioService>();
        auto& node_manager = mAudioService->getNodeManager();

        for (auto& buffer_player_entry : mBufferPlayers)
        {
            for (auto& buffer_player : buffer_player_entry.second)
            {
                auto output_pins = buffer_player.second->getOutputPins();
                for (int i = 0; i<output_pins.size(); i++)
                {
                    assert(i<mMixNodes.size());
                    mMixNodes[i]->inputs.disconnect(*output_pins[i]);
                }
            }
        }

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


    void SequencePlayerAudioOutput::connectInputPin(audio::InputPin& inputPin, int channel)
    {
        assert(channel<mMaxChannels);
        inputPin.connect(mMixNodes[channel]->audioOutput);
    }


    void SequencePlayerAudioOutput::disconnectInputPin(audio::InputPin& inputPin, int channel)
    {
        assert(channel<mMaxChannels);
        inputPin.disconnect(mMixNodes[channel]->audioOutput);
    }


    const std::vector<rtti::ObjectPtr<audio::AudioBufferResource>>& SequencePlayerAudioOutput::getBuffers() const
    {
        return mAudioBuffers;
    }


    audio::OutputPin* SequencePlayerAudioOutput::getOutputForChannel(int channel)
    {
        assert(channel<mMaxChannels);
        return &mMixNodes[channel]->audioOutput;
    }


    int SequencePlayerAudioOutput::getChannelCount() const
    {
        return mMaxChannels;
    }
}
