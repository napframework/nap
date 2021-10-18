/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudiooutputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/node/outputnode.h>
#include <audio/node/pullnode.h>
#include <audio/component/audiocomponentbase.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::SequencePlayerAudioOutputComponent)
        RTTI_PROPERTY("Sequence Player Audio Output", &nap::audio::SequencePlayerAudioOutputComponent::mSequencePlayerAudioOutput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::SequencePlayerAudioOutputComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
RTTI_END_CLASS

namespace nap
{
    namespace audio
    {

        bool SequencePlayerAudioOutputComponentInstance::init(utility::ErrorState &errorState)
        {
            /**
             * Create OutputNode for each channel of SequencePlayerAudioOutput and connect pins with SequencePlayerAudioOutput
             */
            auto *audio_service = getEntityInstance()->getCore()->getService<AudioService>();
            auto &node_manager = audio_service->getNodeManager();

            auto *resource = getComponent<SequencePlayerAudioOutputComponent>();
            mSequencePlayerAudioOutput = resource->mSequencePlayerAudioOutput.get();

            for (int channel = 0; channel < mSequencePlayerAudioOutput->mMaxChannels; channel++)
            {
                auto output_node = node_manager.makeSafe<OutputNode>(node_manager);
                mSequencePlayerAudioOutput->connectInputPin(output_node->audioInput, channel);
                mOutputNodes.emplace_back(std::move(output_node));
            }

            return true;
        }


        void SequencePlayerAudioOutputComponentInstance::onDestroy()
        {
            for (int channel = 0; channel < mSequencePlayerAudioOutput->mMaxChannels; channel++)
            {
                assert(channel < mOutputNodes.size());
                mSequencePlayerAudioOutput->disconnectInputPin(mOutputNodes[channel]->audioInput, channel);
            }
        }
    }
}
