#include "DryWetEnable.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::DryWetEnableNode)
    RTTI_PROPERTY("dryInput", &nap::audio::DryWetEnableNode::dryInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("wetInput", &nap::audio::DryWetEnableNode::wetInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setEnabled", &nap::audio::DryWetEnableNode::setEnabled)
    RTTI_FUNCTION("setDry", &nap::audio::DryWetEnableNode::setDry)
    RTTI_FUNCTION("setWet", &nap::audio::DryWetEnableNode::setWet)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::DryWetEnable)
    RTTI_PROPERTY("DryInput", &nap::audio::DryWetEnable::mDryInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("WetInput", &nap::audio::DryWetEnable::mWetInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        DryWetEnableNode::DryWetEnableNode(NodeManager& manager) : Node(manager)
        {
            mDry.setStepCount(manager.getSamplesPerMillisecond() * 10);
            mWet.setStepCount(manager.getSamplesPerMillisecond() * 10);
        }

        
        void DryWetEnableNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            
            if (mDry.isRamping())
            {
                if (mWet.isRamping())
                {
                    auto& dryBuffer = *dryInput.pull();
                    auto& wetBuffer = *wetInput.pull();
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = dryBuffer[i] * mDry.getNextValue() + wetBuffer[i] * mWet.getNextValue();
                }
                else {
                    auto wet = mWet.getValue();
                    if (wet == 0)
                    {
                        auto& dryBuffer = *dryInput.pull();
                        for (auto i = 0; i < outputBuffer.size(); ++i)
                            outputBuffer[i] = dryBuffer[i] * mDry.getNextValue();
                    }
                    else {
                        auto& dryBuffer = *dryInput.pull();
                        auto& wetBuffer = *wetInput.pull();
                        for (auto i = 0; i < outputBuffer.size(); ++i)
                            outputBuffer[i] = dryBuffer[i] * mDry.getNextValue() + wetBuffer[i] * wet;
                    }
                }
            }
            else {
                auto dry = mDry.getValue();
                if (dry == 0)
                {
                    if (mWet.isRamping())
                    {
                        auto& wetBuffer = *wetInput.pull();
                        for (auto i = 0; i < outputBuffer.size(); ++i)
                            outputBuffer[i] = wetBuffer[i] * mWet.getNextValue();
                    }
                    else {
                        auto wet = mWet.getValue();
                        if (wet == 0)
                        {
                            for (auto i = 0; i < outputBuffer.size(); ++i)
                                outputBuffer[i] = 0;
                        }
                        else {
                            auto& wetBuffer = *wetInput.pull();
                            for (auto i = 0; i < outputBuffer.size(); ++i)
                                outputBuffer[i] = wetBuffer[i] * wet;
                        }
                    }
                }
                else {
                    if (mWet.isRamping())
                    {
                        auto& wetBuffer = *wetInput.pull();
                        auto& dryBuffer = *dryInput.pull();
                        for (auto i = 0; i < outputBuffer.size(); ++i)
                            outputBuffer[i] = dryBuffer[i] * dry + wetBuffer[i] * mWet.getNextValue();
                    }
                    else {
                        auto wet = mWet.getValue();
                        if (wet == 0)
                        {
                            auto& dryBuffer = *dryInput.pull();
                            for (auto i = 0; i < outputBuffer.size(); ++i)
                                outputBuffer[i] = dryBuffer[i] * dry;
                        }
                        else {
                            auto& wetBuffer = *wetInput.pull();
                            auto& dryBuffer = *dryInput.pull();
                            for (auto i = 0; i < outputBuffer.size(); ++i)
                                outputBuffer[i] = dryBuffer[i] * dry + wetBuffer[i] * wet;
                        }
                    }
                }
            }
        }
        
        
        bool DryWetEnable::initNode(int channel, DryWetEnableNode& node, utility::ErrorState& errorState)
        {
            if(mDryInput != nullptr)
                node.dryInput.connect(*mDryInput->getInstance()->getOutputForChannel(channel % mDryInput->getInstance()->getChannelCount()));
            
            if(mWetInput != nullptr)
                node.wetInput.connect(*mWetInput->getInstance()->getOutputForChannel(channel % mWetInput->getInstance()->getChannelCount()));
            
            return true;
        }
        
    }
    
}
