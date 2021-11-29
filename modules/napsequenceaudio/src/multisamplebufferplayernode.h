/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>
=
// Nap includes
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/node/bufferplayernode.h>

namespace nap
{
    namespace audio
    {
        /**
         * The MultiSampleBufferPlayerNode plays back the sample from a MultiSampleBuffer.
         * They key difference between the MultiSampleBufferPlayerNode and the BufferPlayerNode is that the MultiSampleBufferPlayerNode
         * creates OutputPins for each channel of the MultiSampleBuffer
         */
        class NAPAPI MultiSampleBufferPlayerNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            MultiSampleBufferPlayerNode(int channel, NodeManager& manager);

            /**
             * The output to connect to other nodes
             */
            std::vector<OutputPin*> getOutputPins();

            /**
             * Tells the node to start playback
             * @param position: the starting position in the source buffer in samples
             * @param speed: the playbackspeed, 1.0 means 1 sample per sample, 2 means double speed, etc.
             */
            void play(DiscreteTimeValue position = 0, ControllerValue speed = 1.);

            /**
             * Stops playback
             */
            void stop();

            /**
             * Set the playback speed
             * @param speed as a fraction of the original speed of the audio material in the buffer.
             */
            void setSpeed(ControllerValue speed);

            /**
             * Sets the current position of playback while playing.
             * @param position in samples
             */
            void setPosition(DiscreteTimeValue position);

            /**
             * Sets the buffer to be played back from. Can't be called while playing!
             * @param buffer SafePtr to a multichannel sample buffer
             */
            void setBuffer(SafePtr<MultiSampleBuffer> buffer);

            /**
             * @return the playback speed as a fraction of the original speed of the audio material in the buffer.
             */
            ControllerValue getSpeed() const { return mSpeed; }

            /**
             * @return the current playback position within the source buffer.
             */
            DiscreteTimeValue getPosition() const { return mPosition; }
        private:
            // Inherited from Node
            void process() override;

            std::atomic<bool> mPlaying          = {false}; // Indicates wether the node is currently playing.
            std::atomic<int> mChannels          = {0}; // The amount of channels
            std::atomic<double> mPosition       = {0}; // Current position of playback in samples within the source buffer.
            std::atomic<ControllerValue> mSpeed = {1.f}; // Playback speed as a fraction of the original speed.
            SafePtr<MultiSampleBuffer> mBuffer  = nullptr; // Pointer to the buffer with audio material being played back.

            std::vector<std::unique_ptr<OutputPin>> mOwnedOutputPins;
        };

    }
}
