#include "Switch.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::SwitchNode)
    RTTI_PROPERTY("audioOutput", &nap::audio::SwitchNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("select", &nap::audio::SwitchNode::select)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::Switch)
    RTTI_PROPERTY("OnInput", &nap::audio::Switch::mOnInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("OffInput", &nap::audio::Switch::mOffInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("On", &nap::audio::Switch::mOn, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        SwitchNode::SwitchNode(NodeManager& manager) : Node(manager)
        {
            mBalance.setStepCount(manager.getSamplesPerMillisecond() * 10.f);
        }        

        
        void SwitchNode::select(bool on)
        {
            if (on == mOn)
                return;
            
            mOn = on;
            if (mOn)
                mBalance.setValue(1.f);
            else
                mBalance.setValue(0.f);
        }

        
        void SwitchNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            if (mBalance.isRamping())
            {
                auto& onBuffer = *onInput.pull();
                auto& offBuffer = *offInput.pull();
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    auto balance = mBalance.getNextValue();
                    outputBuffer[i] = offBuffer[i] * (1.f - balance) + onBuffer[i] * balance;
                }
            }
            else {
                if (mBalance.getValue() <= 0)
                {
                    auto& offBuffer = *offInput.pull();
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = offBuffer[i];
                }
                else if (mBalance.getValue() >= 1.f)
                {
                    auto& onBuffer = *onInput.pull();
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = onBuffer[i];
                }
                else {
                    auto& onBuffer = *onInput.pull();
                    auto& offBuffer = *offInput.pull();
                    auto balanceOn = mBalance.getValue();
                    auto balanceOff = 1.f - balanceOn;
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = offBuffer[i] * balanceOff + onBuffer[i] * balanceOn;
                }
            }
        }
        
        
        bool Switch::initNode(int channel, SwitchNode& node, utility::ErrorState& errorState)
        {
            node.onInput.connect(*mOnInput->getInstance()->getOutputForChannel(channel % mOnInput->getInstance()->getChannelCount()));
            node.offInput.connect(*mOffInput->getInstance()->getOutputForChannel(channel % mOffInput->getInstance()->getChannelCount()));
            node.select(mOn);
            
            return true;
        }
        
    }
    
}
