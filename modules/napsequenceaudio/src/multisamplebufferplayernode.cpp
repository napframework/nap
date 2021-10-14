/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "multisamplebufferplayernode.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiSampleBufferPlayerNode)
//RTTI_PROPERTY("audioOutputs", &nap::audio::MultiSampleBufferPlayerNode::audioOutputs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_FUNCTION("play", &nap::audio::MultiSampleBufferPlayerNode::play)
RTTI_FUNCTION("stop", &nap::audio::MultiSampleBufferPlayerNode::stop)
RTTI_FUNCTION("setSpeed", &nap::audio::MultiSampleBufferPlayerNode::setSpeed)
RTTI_FUNCTION("setPosition", &nap::audio::MultiSampleBufferPlayerNode::setPosition)
RTTI_END_CLASS

namespace nap
{
    namespace audio
    {
        MultiSampleBufferPlayerNode::MultiSampleBufferPlayerNode(int channels, NodeManager &manager) : Node(manager)
        {
            mChannels.store(channels);

            for(int i = 0 ; i < channels; i++)
            {
                mOwnedOutputPins.emplace_back(std::make_unique<OutputPin>(this));
            }
        }


        std::vector<OutputPin*> MultiSampleBufferPlayerNode::getOutputPins()
        {
            std::vector<OutputPin*> output_pins;
            for(const auto& output_pin : mOwnedOutputPins)
            {
                output_pins.emplace_back(output_pin.get());
            }
            return output_pins;
        }


        void MultiSampleBufferPlayerNode::play(DiscreteTimeValue position, ControllerValue speed)
        {
            mPlaying = true;
            mPosition = position;
            mSpeed = speed;
        }


        void MultiSampleBufferPlayerNode::stop()
        {
            mPlaying = false;
        }


        void MultiSampleBufferPlayerNode::setSpeed(ControllerValue speed)
        {
            mSpeed = speed;
        }


        void MultiSampleBufferPlayerNode::setPosition(DiscreteTimeValue position)
        {
            mPosition = position;
        }


        void MultiSampleBufferPlayerNode::setBuffer(SafePtr<MultiSampleBuffer> buffer)
        {
            assert(mPlaying == false); // It is not safe to do this while playing back!

            if(buffer->getChannelCount() != mChannels.load())
            {
                nap::Logger::warn("cannot set buffer. expected %i channels but multisample buffer has %i", mChannels.load(), buffer->getChannelCount());
            }
            mBuffer = std::move(buffer);
        }


        void MultiSampleBufferPlayerNode::process()
        {
            auto playing = mPlaying.load();
            auto position = mPosition.load();
            auto speed = mSpeed.load();

            // If we're not playing, fill the buffers with 0's and bail out.
            if (!playing || mBuffer == nullptr)
            {
                for(auto& output : mOwnedOutputPins)
                {
                    auto& outputBuffer = getOutputBuffer(*output);
                    std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
                }
                return;
            }

            int channel = 0;
            for(auto& output : mOwnedOutputPins)
            {
                auto& outputBuffer = getOutputBuffer(*output);

                DiscreteTimeValue flooredPosition;
                SampleValue lastValue, newValue, fractionalPart;
                SampleBuffer& channelBuffer = (*mBuffer)[channel];

                // For each sample
                for (auto i = 0; i < outputBuffer.size(); i++) {
                    // Have we reached the destination?
                    if (position + 1 >= channelBuffer.size()) {
                        outputBuffer[i] = 0;
                        if (playing)
                            playing = false;
                    } else {
                        flooredPosition = DiscreteTimeValue(position);
                        lastValue = channelBuffer[flooredPosition];
                        newValue = channelBuffer[flooredPosition + 1];

                        fractionalPart = position - flooredPosition;

                        outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));

                        position += speed;
                    }
                }

                channel++;
            }

            mPosition.store(position);
            mPlaying.store(playing);
        }
    }
}
