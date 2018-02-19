#pragma once

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>

namespace nap {
    
    namespace audio {
        
        class NAPAPI FilterNode : public Node {
        public:
            enum class Mode { LOWPASS, HIGHPASS, BANDPASS, LOWRES, HIGHRES };
            
        public:
            FilterNode(NodeManager& nodeManager) : Node(nodeManager), mOutput(8), mInput(8) { }
            
            // Inherited from Node
            void process() override;
            
            /**
             * The input to be filtered
             */
            InputPin audioInput;
            
            /**
             * Outputs the filtered signal
             */
            OutputPin audioOutput = { this };
            
            void setMode(Mode mode);
            void setFrequency(ControllerValue cutoffFrequency);
            void setResonance(ControllerValue resonance);
            void setBand(ControllerValue band);
            void setGain(ControllerValue gain);
            
            Mode getMode() const { return mMode; }
            ControllerValue getFrequency() const { return mFrequency; }
            ControllerValue getResonance() const { return mResonance; }
            ControllerValue getBand() const { return mBand; }
            ControllerValue getGain() const { return mGain; }

        private:
            void adjust();
            
            Mode mMode = Mode::LOWPASS;
            ControllerValue mFrequency = 440.f;
            ControllerValue mResonance = 0.f;
            ControllerValue mBand = 100.f;
            ControllerValue mGain = 1.f;
            
            ControllerValue a0, a1, a2, b1, b2;
            
            Delay mOutput;
            Delay mInput;
        };
        
    }
    
}
