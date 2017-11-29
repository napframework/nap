#include "levelmeternode.h"

#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::LevelMeterNode)
    RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
    RTTI_FUNCTION("getLevel", &nap::audio::LevelMeterNode::getLevel)
RTTI_END_CLASS



namespace nap {
    
    namespace audio {
        
        LevelMeterNode::LevelMeterNode(NodeManager& manager, TimeValue analysisWindowSize) : Node(manager)
        {
            mBuffer.resize(getNodeManager().getSamplesPerMillisecond() * analysisWindowSize);
            manager.registerRootNode(*this);
        }
        
        
        LevelMeterNode::~LevelMeterNode()
        {
            getNodeManager().unregisterRootNode(*this);
        }
        
        
        void LevelMeterNode::setAnalysisWindowSize(TimeValue size)
        {
            mIndex = 0;
            mBuffer.resize(getNodeManager().getSamplesPerMillisecond() * size);
        }
        
        
        void LevelMeterNode::process()
        {
            auto inputBuffer = input.pull();
            
            for (auto& sample : *inputBuffer)
            {
                mBuffer[mIndex++] = sample;
                if (mIndex == mBuffer.size())
                {
                    mIndex = 0;
                    switch (mType) {
                        case PEAK:
                            mValue = calculatePeak();
                            break;
                            
                        default:
                            mValue = calculateRms();
                            break;
                    }
                }
            }
        }
        
        
        float LevelMeterNode::calculateRms()
        {
            float x = 0;
            for (auto i = 0; i < mBuffer.size(); ++i)
                x += mBuffer[i] * mBuffer[i];
            x /= float(mBuffer.size());
            
            return sqrt(x);
        }
        
        
        float LevelMeterNode::calculatePeak()
        {
            float x = 0;
            for (auto i = 0; i < mBuffer.size(); ++i)
            {
                float newValue = fabs(mBuffer[i]);
                if (newValue > x)
                    x = newValue;
            }
            return x;
        }
        
    }
}
