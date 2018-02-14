#include "stereopanner.h"
#include <cmath>

namespace nap
{
    
    namespace audio
    {
        
        StereoPanner::StereoPanner(NodeManager& manager) : Node(manager)
        {
            setPanning(mPanning);
        }
        
        
        void StereoPanner::setPanning(ControllerValue value)
        {
            mPanning = value;
            mLeftGain = cos(mPanning * 0.5 * M_PI);
            mRightGain = sin(mPanning * 0.5 * M_PI);
        }
        
        
        void StereoPanner::process()
        {
            auto& leftInputBuffer = *leftInput.pull();
            auto& rightInputBuffer = *rightInput.pull();
            auto& leftOutputBuffer = getOutputBuffer(leftOutput);
            auto& rightOutputBuffer = getOutputBuffer(rightOutput);
            
            for (auto i = 0; i < leftOutputBuffer.size(); ++i)
                leftOutputBuffer[i] = leftInputBuffer[i] * mLeftGain;
            
            for (auto i = 0; i < rightOutputBuffer.size(); ++i)
                rightOutputBuffer[i] = rightInputBuffer[i] * mRightGain;
        }
        
    }
    
}
