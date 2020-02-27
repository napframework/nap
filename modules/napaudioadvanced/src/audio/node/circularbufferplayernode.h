#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/node/circularbuffernode.h>
#include <audio/utility/safeptr.h>
#include <audio/utility/dirtyflag.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Node to play back audio from a circular buffer
         */
        class NAPAPI CircularBufferPlayerNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            CircularBufferPlayerNode(NodeManager& manager) : Node(manager) { }
        
            /**
             * The output to connect to other nodes
             */
            OutputPin audioOutput = { this  };
            
            /**
             * Tells the node to start playback
             * @param buffer: the circular buffer to play audio from
             * @param relativePosition: the starting position in the source buffer in samples, relative to the current write position of the circular buffer.
             * @param speed: the playbackspeed, 1.0 means 1 sample per sample, 2 means double speed, etc.
             */
            void play(CircularBufferNode& buffer, int relativePosition = 0, ControllerValue speed = 1.);
            
            /**
             * Stops playback
             */
            void stop();
            
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
  
            double mPosition = 0; // Current position of playback in samples within the source buffer.
            ControllerValue mSpeed = 1.f; // Playback speed as a fraction of the original speed.
            CircularBufferNode* mBuffer = nullptr; // Pointer to the circular buffer that is used as source playback material.
            
            std::atomic<CircularBufferNode*> mNewBuffer = { nullptr };
            std::atomic<int> mNewRelativePosition = { 0 };
            std::atomic<ControllerValue> mNewSpeed = { 1.f };
            DirtyFlag mIsDirty;
        };
        
    }
}
