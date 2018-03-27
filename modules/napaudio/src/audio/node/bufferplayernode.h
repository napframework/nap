#pragma once

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
             * @param position: the starting position in the source buffer in samples
             * @param speed: the playbackspeed, 1.0 means 1 sample per sample, 2 means double speed, etc.
             */
            void play(std::shared_ptr<SampleBuffer>& buffer, DiscreteTimeValue position = 0, ControllerValue speed = 1.);
            
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
             * @return: the playback speed as a fraction of the original speed of the audio material in the buffer.
             */
            ControllerValue getSpeed() const { return mSpeed; }
            
            /**
             * @return: the current playback position within the source buffer.
             */
            DiscreteTimeValue getPosition() const { return mPosition; }
            
        private:
            // Inherited from Node
            void process() override;
  
            bool mPlaying = false; // Indicates wether the node is currently playing.
            long double mPosition = 0; // Current position of playback in samples within the source buffer.
            ControllerValue mSpeed = 1.f; // Playback speed as a fraction of the original speed.
            std::shared_ptr<SampleBuffer> mBuffer = nullptr; // Pointer to the buffer with audio material being played back. The pointer is shared because audio nodes destruction is always deferred until the next audio callback
        };
        
    }
}
