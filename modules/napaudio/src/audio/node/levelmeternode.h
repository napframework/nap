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
            
            float calculateRms(); // Calculates output value out of one buffer of data using root mean square algorithm
            float calculatePeak(); // Calculates output value out of one buffer of data by determining the maximum amplitude of the buffer.
            
            SampleBuffer mBuffer; // Buffer being analyzed
            std::atomic<float> mValue = { 0 }; // Output level value stored atomically so it can be queried safely from different threads.
            int mIndex = 0; // Current write index of the buffer being analyzed.
            Type mType = Type::RMS; // Algorithm currently being used to calculate the output level value from one buffer.
            
        };
        
    }
    
}
