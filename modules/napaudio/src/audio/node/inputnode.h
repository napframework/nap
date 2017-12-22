#pragma once

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
    
        
        /**
         * This node outputs the audio input that is received from the node system's external input, typically an audio interface.
         * Input from channel @inputChannel can be pulled from @audioOutput plug.
         */
        class NAPAPI InputNode final : public Node
        {
            friend class NodeManager;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
             */
            InputNode(NodeManager& manager) : Node(manager) { }
            
            /**
             * This output will contain the samples received from the node system's external input.
             */
            OutputPin audioOutput = { this };
            
            /**
             * Sets the channel from which this node receives input.
             */
            void setInputChannel(int inputChannel) { mInputChannel = inputChannel; }
            
            /**
             * @return: the channel from which this node receives input.
             */
            int getInputChannel() const { return mInputChannel; }
            
        private:
            void process() override;
            
            int mInputChannel = 0;
        };
        
        
    }
}





