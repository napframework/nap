#pragma once

#include <nap/resource.h>

#include <audio/core/nodeobject.h>


namespace nap
{
    namespace audio
    {
        
        /**
         * This class exists so we can RTTI define the 'input' resourceptr of MultiChannelWithInput.
         */
        class NAPAPI InputResourceContainer
        {
            RTTI_ENABLE()
        public:
            InputResourceContainer() = default;
			
            virtual ~InputResourceContainer() = default;
            
            ResourcePtr<AudioObject> mInput = nullptr;
        };
        
        /**
         * MultiChannel audioobject with a single input property baked in.
         */
        template <typename NodeType>
        class NAPAPI MultiChannelWithInput : public ParallelNodeObject<NodeType>, public InputResourceContainer
        {
            RTTI_ENABLE(ParallelNodeObjectBase, InputResourceContainer)
            
        public:
            MultiChannelWithInput() = default;
            
            
        private:
            /**
             * initNode sets the input. To set custom properties, 'onInitNode' needs to be overriden by descendants.
             */
            virtual bool initNode(int channel, NodeType& node, utility::ErrorState& errorState) override final {
                
                // mInput is allowed to stay nullptr, then the input will simply not be connected.
                if(mInput != nullptr)
                    (*node.getInputs().begin())->connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
                
                return onInitNode(channel, node, errorState);
            }
            
            // should be renamed if 'initNode' will be renamed
            virtual bool onInitNode(int channel, NodeType& node, utility::ErrorState& errorState) { return true; };
        };
        
    }
}
