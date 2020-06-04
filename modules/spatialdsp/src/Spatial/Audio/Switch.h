#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <rtti/rtti.h>
#include <mathutils.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/audiotypes.h>
#include <audio/utility/linearsmoothedvalue.h>
#include <audio/core/nodeobject.h>

namespace nap
{
    
    namespace audio
    {
        
        
        /**
         * Node that switches between two signals (typically a dry and wet signal) called "on" and "off" in order to switch an effect on or off.
         */
        class NAPAPI SwitchNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            SwitchNode(NodeManager& manager);

            OutputPin audioOutput = { this };
            InputPin onInput = { this }; ///< The input pin with the "on" signal.
            InputPin offInput = { this }; ///< The input pin with the "off" signal.
            
            /**
             * Select the "on" or the "off" input.
             */
            void select(bool on);
            
        private:
            void process() override;            
            bool mOn = false;
            LinearSmoothedValue<ControllerValue> mBalance = { 0, 64 };
        };
        
        
        /**
         * @MultiChannelObject containing SwitchNode nodes for each channel.
         */
        class NAPAPI Switch : public ParallelNodeObject<SwitchNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            Switch() = default;
            
            ResourcePtr<AudioObject> mOnInput; ///< Property 'OnInput'
            ResourcePtr<AudioObject> mOffInput; ///< Proeprty 'OffInput'
            int mChannelCount = 1; ///< Property 'ChannelCount' Number of channels
            bool mOn = false; ///< Property 'On'

        private:
            virtual bool initNode(int channel, SwitchNode& node, utility::ErrorState& errorState) override;
            
        };
        
    }
    
}
