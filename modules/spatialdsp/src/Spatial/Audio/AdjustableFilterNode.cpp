#include "AdjustableFilterNode.h"

// Audio includes
#include <audio/core/audionodemanager.h>


RTTI_BEGIN_ENUM(nap::audio::AdjustableFilterNode::EMode)
    RTTI_ENUM_VALUE(nap::audio::AdjustableFilterNode::EMode::LowPass, "LowPass"),
    RTTI_ENUM_VALUE(nap::audio::AdjustableFilterNode::EMode::HighPass, "HighPass"),
    RTTI_ENUM_VALUE(nap::audio::AdjustableFilterNode::EMode::BandPass, "BandPass"),
    RTTI_ENUM_VALUE(nap::audio::AdjustableFilterNode::EMode::LowRes, "LowRes"),
    RTTI_ENUM_VALUE(nap::audio::AdjustableFilterNode::EMode::HighRes, "HighRes")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AdjustableFilterNode)
    RTTI_PROPERTY("audioInput", &nap::audio::AdjustableFilterNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::AdjustableFilterNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setMode", &nap::audio::AdjustableFilterNode::setMode)
    RTTI_FUNCTION("setFrequency", &nap::audio::AdjustableFilterNode::setFrequency)
    RTTI_FUNCTION("setResonance", &nap::audio::AdjustableFilterNode::setResonance)
    RTTI_FUNCTION("setBand", &nap::audio::AdjustableFilterNode::setBand)
    RTTI_FUNCTION("setGain", &nap::audio::AdjustableFilterNode::setGain)
    RTTI_FUNCTION("getMode", &nap::audio::AdjustableFilterNode::getMode)
    RTTI_FUNCTION("getFrequency", &nap::audio::AdjustableFilterNode::getFrequency)
    RTTI_FUNCTION("getResonance", &nap::audio::AdjustableFilterNode::getResonance)
    RTTI_FUNCTION("getBand", &nap::audio::AdjustableFilterNode::getBand)
    RTTI_FUNCTION("getGain", &nap::audio::AdjustableFilterNode::getGain)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        void AdjustableFilterNode::process()
        {
            if (mIsDirty.check())
                update();
            
            auto inputBuffer = audioInput.pull();
            auto& outputBuffer = getOutputBuffer(audioOutput);
            
            if (inputBuffer)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mInput.write((*inputBuffer)[i]);
                    auto temp = a0.getNextValue() * mInput.read(0) + a1.getNextValue() * mInput.read(1) + a2.getNextValue() * mInput.read(2) - b1.getNextValue() * mOutput.read(0) - b2.getNextValue() * mOutput.read(1);
                    
                    mOutput.write(temp);
                    outputBuffer[i] = temp;
                }
            }
            else {
                // process with 0 input
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mInput.write(0);
                    auto temp = a0.getNextValue() * mInput.read(0) + a1.getNextValue() * mInput.read(1) + a2.getNextValue() * mInput.read(2) - b1.getNextValue() * mOutput.read(0) - b2.getNextValue() * mOutput.read(1);
                    
                    mOutput.write(temp);
                    outputBuffer[i] = temp;
                }
            }
        }
        
        
        void AdjustableFilterNode::setMode(EMode mode)
        {
            mMode = mode;
            mIsDirty.set();
        }
        
        
        void AdjustableFilterNode::setFrequency(ControllerValue frequency)
        {
            mFrequency = frequency;
            if (mFrequency <= 0)
                mFrequency = 1;
            mIsDirty.set();
        }

        
        void AdjustableFilterNode::setResonance(ControllerValue resonance)
        {
            mResonance = pow(10., - (resonance * 0.1));
            mIsDirty.set();
        }
        
        
        void AdjustableFilterNode::setBand(ControllerValue band)
        {
            mBand = band;
            if (mBand <= 0)
                mBand = 1;
            mIsDirty.set();
        }
        
        
        void AdjustableFilterNode::setGain(ControllerValue gain)
        {
            mGain = gain;
            mIsDirty.set();
        }
        
        
        void AdjustableFilterNode::setRampTime(TimeValue rampTime)
        {
            int stepCount = rampTime * getNodeManager().getSamplesPerMillisecond();
            a0.setStepCount(stepCount);
            a1.setStepCount(stepCount);
            a2.setStepCount(stepCount);
            b1.setStepCount(stepCount);
            b2.setStepCount(stepCount);
        }

        
        void AdjustableFilterNode::update()
        {
            ControllerValue c, d, cSqr, q;
            ControllerValue _a0, _a1, _a2, _b1, _b2;
            switch (mMode)
            {
                case EMode::LowPass:
                    c = 1 / tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    _a0 = (1 / (1 + M_SQRT2 * c + cSqr));
                    _a1 = 2 * _a0;
                    _a2 = _a0;
                    _b1 = 2 * _a0 * (1 - cSqr);
                    _b2 = _a0 * (1 - M_SQRT2 * c + cSqr);
                    break;
                case EMode::HighPass:
                    c = tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    _a0 = 1 / (1 + M_SQRT2 * c + cSqr);
                    _a1 = -2 * _a0;
                    _a2 = _a0;
                    _b1 = 2 * _a0 * (cSqr - 1);
                    _b2 = _a0 * (1 - M_SQRT2 * c + cSqr);
                    break;
                case EMode::BandPass:
                    c = 1 / tan(M_PI * mBand / getSampleRate());
                    d = 2 * cos(2 * M_PI * mFrequency / getSampleRate());
                    _a0 = 1 / (1 + c);
                    _a1 = 0;
                    _a2 = -_a0;
                    _b1 = _a2 * c * d;
                    _b2 = _a0 * (c - 1);
                    break;
                case EMode::LowRes:
                    c = 1 / tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    q = M_SQRT2 * mResonance;
                    _a0 = (1 / (1 + q * c + cSqr));
                    _a1 = 2 * _a0;
                    _a2 = _a0;
                    _b1 = 2 * _a0 * (1 - cSqr);
                    _b2 = _a0 * (1 - q * c + cSqr);
                    break;
                case EMode::HighRes:
                    c = tan(M_PI * mFrequency / getSampleRate());
                    cSqr = c * c;
                    q = M_SQRT2 * mResonance;
                    _a0 = 1 / (1 + q * c + cSqr);
                    _a1 = -2 * _a0;
                    _a2 = _a0;
                    _b1 = 2 * _a0 * (cSqr - 1);
                    _b2 = _a0 * (1 - q * c + cSqr);
                    break;
            }
            _a0 *= mGain;
            _a1 *= mGain;
            _a2 *= mGain;
            a0.setValue(_a0);
            a1.setValue(_a1);
            a2.setValue(_a2);
            b1.setValue(_b1);
            b2.setValue(_b2);
        }

        
    }
    
}
