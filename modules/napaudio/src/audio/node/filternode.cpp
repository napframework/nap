#include "filternode.h"

namespace nap {
    
    namespace audio {
        
        
        void FilterNode::process()
        {
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(audioOutput);
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                mInput.write(inputBuffer[i]);
                auto temp = a0 * mInput.read(0) + a1 * mInput.read(1) + a2 * mInput.read(2) - b1 * mOutput.read(0) - b2 * mOutput.read(1);
                
//                if (abs(temp) < insignificant())
//                    temp = 0;
                
                mOutput.write(temp);
                outputBuffer[i] = temp;
            }

        }
        
        
        void FilterNode::setMode(Mode mode)
        {
            mMode = mode;
            adjust();
        }
        
        
        void FilterNode::setFrequency(ControllerValue frequency)
        {
            mFrequency = frequency;
            if (mFrequency <= 0)
                mFrequency = 1;
            adjust();
        }

        
        void FilterNode::setResonance(ControllerValue resonance)
        {
            mResonance = pow(10., - (resonance * 0.1));
            adjust();
        }
        
        
        void FilterNode::setBand(ControllerValue band)
        {
            mBand = band;
            if (mBand <= 0)
                mBand = 1;
            adjust();
        }
        
        
        void FilterNode::setGain(ControllerValue gain)
        {
            mGain = gain;
            adjust();
        }
        
        
        void FilterNode::adjust()
        {
            ControllerValue c, d, cSqr, q;
            switch (mMode)
            {
                case Mode::LOWPASS:
                    c = 1 / tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    a0 = (1 / (1 + M_SQRT2 * c + cSqr));
                    a1 = 2 * a0;
                    a2 = a0;
                    b1 = 2 * a0 * (1 - cSqr);
                    b2 = a0 * (1 - M_SQRT2 * c + cSqr);
                    break;
                case Mode::HIGHPASS:
                    c = tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    a0 = 1 / (1 + M_SQRT2 * c + cSqr);
                    a1 = -2 * a0;
                    a2 = a0;
                    b1 = 2 * a0 * (cSqr - 1);
                    b2 = a0 * (1 - M_SQRT2 * c + cSqr);
                    break;
                case Mode::BANDPASS:
                    c = 1 / tan(M_PI * mBand / getSampleRate());
                    d = 2 * cos(2 * M_PI * mFrequency / getSampleRate());
                    a0 = 1 / (1 + c);
                    a1 = 0;
                    a2 = -a0;
                    b1 = a2 * c * d;
                    b2 = a0 * (c - 1);
                    break;
                case Mode::LOWRES:
                    c = 1 / tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    q = M_SQRT2 * mResonance;
                    a0 = (1 / (1 + q * c + cSqr));
                    a1 = 2 * a0;
                    a2 = a0;
                    b1 = 2 * a0 * (1 - cSqr);
                    b2 = a0 * (1 - q * c + cSqr);
                    break;
                case Mode::HIGHRES:
                    c = tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    q = M_SQRT2 * mResonance;
                    a0 = 1 / (1 + q * c + cSqr);
                    a1 = -2 * a0;
                    a2 = a0;
                    b1 = 2 * a0 * (cSqr - 1);
                    b2 = a0 * (1 - q * c + cSqr);
                    break;
            }
            a0 *= mGain;
            a1 *= mGain;
            a2 *= mGain;
        }

        
    }
    
}
