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
         * Node to pull its input without doing anything with it, just to make sure it's processed.
         * This is handy in order to simulate multispeaker applications processing load properly without needing the actual outputs.
         * The PullNode is a root node that will be directly processed by the node manager.
         */
        class NAPAPI PullNode final : public Node
        {
            RTTI_ENABLE(Node)
        public:
            /**
             * @param nodeManager: The @NodeManager this node provides output to.
             * @param active: true if the node is active and being processed from the moment of creation.
             */
            PullNode(NodeManager& nodeManager, bool active = true);
            
            ~PullNode() override final;
            
            /**
             * The processing chain connected to this input will be processed even when not being used for anything.
             */
            InputPin audioInput;
            
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
            
            bool mActive = true;
        };
        
        
    }
}





