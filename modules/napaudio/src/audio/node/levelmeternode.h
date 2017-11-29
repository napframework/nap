#pragma once

#include <audio/core/audionode.h>

namespace nap {
    
    namespace audio {
        
        class NAPAPI LevelMeterNode : public Node {
        public:
            enum Type { PEAK, RMS };
            
            LevelMeterNode(NodeManager& manager, TimeValue analysisWindowSize = 10);
            virtual ~LevelMeterNode();
            
            InputPin input;
            
            float getLevel() const { return mValue; }
            void setAnalysisWindowSize(TimeValue size);
            void setType(Type type) { mType = type; }
            
        private:
            
            void process() override;
            
            float calculateRms();
            float calculatePeak();
            
            SampleBuffer mBuffer;
            std::atomic<float> mValue = { 0 };
            int mIndex = 0;
            Type mType = Type::RMS;
            
        };
        
    }
    
}
