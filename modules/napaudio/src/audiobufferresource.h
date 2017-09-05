#pragma once

// nap includes
#include <rtti/rttiobject.h>

// audio includes
#include "audiotypes.h"

namespace nap {
    
    namespace audio {
        
        class AudioBufferResource : public rtti::RTTIObject {
            RTTI_ENABLE(rtti::RTTIObject)
        public:
            AudioBufferResource() = default;
            
            float getSampleRate() const { return mSampleRate; }
            DiscreteTimeValue getSize() const { return mBuffer.getSize(); }
            unsigned int getChannelCount() const { return mBuffer.getChannelCount(); }
            
            MultiSampleBuffer& getBuffer() { return mBuffer; }
            
        protected:
            MultiSampleBuffer mBuffer;
            float mSampleRate = 0;
        };
        
    }
    
}
