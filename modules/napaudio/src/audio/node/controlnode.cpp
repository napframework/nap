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
            mValue.setValue(value);
        }

        
        ControllerValue ControlNode::getValue() const
        {
            if (mTranslator)
                return mTranslator->translate(mValue.getValue());
            else
                return mValue.getValue();
        }
        
        
        void ControlNode::ramp(ControllerValue destination, TimeValue time, RampMode mode)
        {
            mValue.ramp(destination, time * getNodeManager().getSamplesPerMillisecond(), mode);
        }
        
        
        bool ControlNode::isRamping() const
        {
            return mValue.isRamping();
        }
        
        
        void ControlNode::stop()
        {
            mValue.stop();
        }
        
        
        void ControlNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            
            if (mTranslator)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mValue.step();
                    outputBuffer[i] = mTranslator->translate(mValue.getValue());
                }
            }
            else {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mValue.step();
                    outputBuffer[i] = mValue.getValue();
                }
            }
        }
        
    }
    
}
