#pragma once

// Nap includes
#include <utility/safeptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Node to play back audio from a buffer
         */
        class NAPAPI BufferPlayerNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            BufferPlayerNode(NodeManager& manager) : Node(manager) { }
        
            /**
             * The output to connect to other nodes
             */
            OutputPin audioOutput = { this  };
            
            /**
             * Tells the node to start playback
             * @param buffer: the buffer to play back from
             * @param channel: the channel within the buffer to be played
             * @param position: the starting position in the source buffer in samples
             * @param speed: the playbackspeed, 1.0 means 1 sample per sample, 2 means double speed, etc.
             */
            void play(utility::SafePtr<MultiSampleBuffer> buffer, int channel = 0, DiscreteTimeValue position = 0, ControllerValue speed = 1.);
            
            /**
             * Stops playback
             */
            void stop();
            
            /**
             * Set the playback speed as a fraction of the original speed of the audio material in the buffer.
             */
            void setSpeed(ControllerValue speed);
            
            /**
             * Sets the current position of playback while playing.
             */
            void setPosition(DiscreteTimeValue position);
            
            /**
             * Sets the current channel of playback while playing.
             */
            void setChannel(int channel);
            
            /**
             * @return: the playback speed as a fraction of the original speed of the audio material in the buffer.
             */
            ControllerValue getSpeed() const { return mSpeed; }
            
            /**
             * @return: the current playback position within the source buffer.
             */
            DiscreteTimeValue getPosition() const { return mPosition; }
            
            /**
             * @return: the current playback channel within the source buffer.
             */
            DiscreteTimeValue getChannel() const { return mChannel; }
            
        private:
            // Inherited from Node
            void process() override;
  
            bool mPlaying = false; // Indicates wether the node is currently playing.
            int mChannel = 0; // The channel within the buffer that is being played bacl/
            long double mPosition = 0; // Current position of playback in samples within the source buffer.
            ControllerValue mSpeed = 1.f; // Playback speed as a fraction of the original speed.
            utility::SafePtr<MultiSampleBuffer> mBuffer = nullptr; // Pointer to the buffer with audio material being played back.
        };
        
    }
}
