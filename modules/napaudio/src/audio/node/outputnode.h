#pragma once

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
    
        
        // Forward declarations
        class AudioService;
        
        /**
         * Node to provide audio output for the node manager's audio processing, typically sent to an audio interface.
         * The OutputNode is a root node that will be directly processed by the node manager.
         */
        class NAPAPI OutputNode final : public Node
        {
        public:
            /**
             * @param nodeManager: The @NodeManager this node provides output to.
             * @param active: true if the node is active and being processed from the moment of creation. This can cause glitches if the node tree and it's parameters are still being build.
             */
            OutputNode(NodeManager& nodeManager, bool active = true);
            
            ~OutputNode() override final;
            
            /**
             * Through this input the node receives buffers of audio samples that will be presented to the node manager as output for its audio processing.
             */
            InputPin audioInput;
            
            /**
             * Set the audio channel that this node's input will be played on by the node manager.
             * @param outputChannel: the channel number
             */
            void setOutputChannel(int outputChannel) { mOutputChannel = outputChannel; }
            
            /**
             * @return: the audio channel that this node's input will be played on by the node manager.
             */
            int getOutputChannel() const { return mOutputChannel; }
            
            /**
             * Sets wether the node will be processed by the audio node manager.
             * On creation the node is inactive by default.
             */
            void setActive(bool active) { mActive = true; }
            
            /**
             * @return: true if the node is currently active and thus being processed (triggered) on every callback.
             */
            bool isActive() const { return mActive; }
            
        private:
            void process() override;
            
            int mOutputChannel = 0; // The audio channel that this node's input will be played on by the node manager.
            
            bool mActive = true;
        };
        
        
    }
}





