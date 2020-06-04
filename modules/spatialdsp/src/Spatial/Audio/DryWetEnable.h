#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <rtti/rtti.h>
#include <mathutils.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/core/audionode.h>
#include <audio/utility/audiotypes.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
    
    namespace audio
    {
        
        
        /**
         * Node that can enable and disable a wet signal by switching between the dry and the wet signal and mix the dry and the wet signal.
         * It has two audio inputs, dry and wet, a @setEnabled function and a @setDry and @setWet function.
         */
        class NAPAPI DryWetEnableNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            DryWetEnableNode(NodeManager& manager);
            
            InputPin dryInput = { this }; ///< The dry input signal.
            InputPin wetInput = { this }; ///< The wet input signal.
            OutputPin audioOutput = { this }; ///< The output signal.
            
            /**
             * When called with false the dry input is selected, with true the mix between wet and dry input is selected as output.
             */
            void setEnabled(bool value)
            {
                if (value == mEnabled)
                    return;
                
                mEnabled = value;
                if (mEnabled)
                {
                    mDry.setValue(mDryCache);
                    mWet.setValue(mWetCache);
                }
                else {
                    mDry.setValue(1.f);
                    mWet.setValue(0.f);
                }
            }
            
            /**
             * Sets the dry gain independently of the wet gain.
             */
            void setDry(ControllerValue value)
            {
                mDryCache = value;
                if (mEnabled)
                    mDry.setValue(mDryCache);
            }
            
            /**
             * Sets the wet gain independently of the dry gain.
             */
            void setWet(ControllerValue value)
            {
                mWetCache = value;
                if (mEnabled)
                    mWet.setValue(mWetCache);
            }
            
            /**
             * Sets the dry-wet balance.
             */
            void setDryWet(ControllerValue value)
            {
                setDry(math::clamp<ControllerValue>(1. - value, 0.f, 1.f));
                setWet(math::clamp<ControllerValue>(value, 0.f, 1.f));
            }
            
        private:
            void process() override;
            
            LinearSmoothedValue<ControllerValue> mDry = { 1.0, 64 };
            LinearSmoothedValue<ControllerValue> mWet = { 0, 64 };
            bool mEnabled = false;
            ControllerValue mDryCache = 1.f;
            ControllerValue mWetCache = 0.f;
        };
        
        
        class NAPAPI DryWetEnable : public ParallelNodeObject<DryWetEnableNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
        public:
            
            ResourcePtr<AudioObject> mDryInput = nullptr;
            ResourcePtr<AudioObject> mWetInput = nullptr;
            
        private:
            virtual bool initNode(int channel, DryWetEnableNode& node, utility::ErrorState& errorState) override;
        };
        
    }
    
}
