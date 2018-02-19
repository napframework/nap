#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>

namespace nap {
    
    namespace audio {
        
        /**
         * Node to measure the amplitude level of an audio signal.
         * Can be used for VU meters or envelope followers for example.
         * Can switch between measuring peaks of the signal or the root mean square.
         */
        class NAPAPI LevelMeterNode : public Node {
        public:
            enum Type { PEAK, RMS };

            /**
             * @param manager: the NodeManager this node will be processed on
             * @param analysisWindowSize: the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
             */
            LevelMeterNode(NodeManager& manager, TimeValue analysisWindowSize = 10);
            virtual ~LevelMeterNode();
            
            InputPin input; /**< The input for the audio signal that will be analyzed. */
            
            /**
             * @return: Thhe current level of the analyzed signal.
             */
            float getLevel() const { return mValue; }
            
            /**
             * Set the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
             */
            void setAnalysisWindowSize(TimeValue size);
            
            /**
             * Set the Type of the analysis. PEAK means the highest absolute amplitude within the analyzed window will be output. RMS means the root mean square of all values within the analyzed window will be output.
             */
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
