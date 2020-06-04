#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/delaynode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a delay to the input channels.
         */
        class NAPAPI DelayObject : public ParallelNodeObject<DelayNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            DelayObject() = default;
            
            std::vector<TimeValue> mTime = { 0.f }; ///< property: 'Time' array of delay time values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<ControllerValue> mFeedback = { 0.f }; ///< property: 'Time' array of feedback values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<TimeValue> mDryWet = { 0.f }; ///< property: 'DryWet' array of dry wet balance levels per output channel. If the size of the array is less than the number of channels it will be repeated.
            ResourcePtr<AudioObject> mInput; ///< property: "Input" @AudioObject whose output channels will be used as inputs for the delay channels.
            
        private:
            bool initNode(int channel, DelayNode& node, utility::ErrorState& errorState) override;
        };
        
        
    }
    
}

