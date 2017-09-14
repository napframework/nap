#include "delaynode.h"

#include <utility/audiofunctions.h>

namespace nap {
    
    namespace audio {

        void DelayNode::process()
        {
            auto& inputBuffer = *input.pull();
            auto& outputBuffer = getOutputBuffer(output);
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                auto delaydSample = mDelay.read(mTime);
                mDelay.write(inputBuffer[i] + delaydSample * mFeedback);
                outputBuffer[i]  = lerp(inputBuffer[i], delaydSample, mDryWet);
            }
        }

    }
    
}
