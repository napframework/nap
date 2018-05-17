#include "controlnode.h"

namespace nap
{
    
    namespace audio
    {
        
        ControlNode::ControlNode(NodeManager& manager) : Node(manager)
        {
            mValue.destinationReachedSignal.connect(mDestinationReachedSlot);
        }
        
        
        void ControlNode::setValue(ControllerValue value)
        {
            mValue.ramp(value, 0);
        }

        
        ControllerValue ControlNode::getValue() const
        {
            
            if (mTranslator != nullptr)
                return mTranslator->translate(mCurrentValue.load());
            else
                return mCurrentValue.load();
        }
        
        
        void ControlNode::ramp(ControllerValue destination, TimeValue time, RampMode mode)
        {
            mValue.ramp(destination, time * getNodeManager().getSamplesPerMillisecond(), mode);
        }
        
        
        void ControlNode::stop()
        {
            mValue.stop();
        }
        
        
        void ControlNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            
            if (mTranslator != nullptr)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = mTranslator->translate(mValue.getNextValue());
                }
            }
            else {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = mValue.getNextValue();
                }
            }
            mCurrentValue.store(mValue.getValue());
        }
        
    }
    
}
