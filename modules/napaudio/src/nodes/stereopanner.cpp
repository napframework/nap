#include "stereopanner.h"

// Std includes
#include <math.h>

namespace nap {
    
    namespace audio {
        
        StereoPanner::StereoPanner(AudioNodeManager& manager) : AudioNode(manager)
        {
            setPanning(mPanning);
        }
        
        
        void StereoPanner::setPanning(ControllerValue value)
        {
            mPanning = value;
            mLeftGain = cos(mPanning * 0.5 * pi);
            mRightGain = sin(mPanning * 0.5 * pi);
        }
        
        
        void StereoPanner::calculateLeft(SampleBuffer& buffer)
        {
            SampleBuffer& inputBuffer = *leftInput.pull();
            
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = inputBuffer[i] * mLeftGain;
        }
        
        
        void StereoPanner::calculateRight(SampleBuffer& buffer)
        {
            SampleBuffer& inputBuffer = *rightInput.pull();
            
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = inputBuffer[i] * mRightGain;
        }
        

    }
    
}
