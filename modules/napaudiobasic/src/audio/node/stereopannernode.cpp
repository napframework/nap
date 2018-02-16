#include "stereopannernode.h"
#include <audio/utility/audiofunctions.h>

namespace nap
{
    
    namespace audio
    {
        
        StereoPannerNode::StereoPannerNode(NodeManager& manager) : Node(manager)
        {
            setPanning(mPanning);
        }
        
        
        void StereoPannerNode::setPanning(ControllerValue value)
        {
            mPanning = value;
            equalPowerPan<ControllerValue>(mPanning, mLeftGain, mRightGain);
        }
        
        
        void StereoPannerNode::process()
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
