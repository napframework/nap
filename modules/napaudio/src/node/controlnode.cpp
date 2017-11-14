#include "controlnode.h"

namespace nap
{
    
    namespace audio
    {
        
        ControlNode::ControlNode(NodeManager& manager) : Node(manager)
        {
            mLinearRamper.destinationReachedSignal.connect(mDestinationReachedSlot);
            mExponentialRamper.destinationReachedSignal.connect(mDestinationReachedSlot);
        }
        
        
        void ControlNode::setValue(ControllerValue value)
        {
            mLinearRamper.stop();
            mExponentialRamper.stop();
            mValue = value;
        }

        
        ControllerValue ControlNode::getValue() const
        {
            if (mTranslator)
                return mTranslator->translate(mValue);
            else
                return mValue;
        }
        
        
        void ControlNode::ramp(ControllerValue destination, TimeValue time, RampMode mode)
        {
            switch (mode) {
                case RampMode::LINEAR:
                    mExponentialRamper.stop();
                    mLinearRamper.ramp(destination, time * getNodeManager().getSamplesPerMillisecond());
                    break;
                    
                case RampMode::EXPONENTIAL:
                    mLinearRamper.stop();
                    mExponentialRamper.ramp(destination, time * getNodeManager().getSamplesPerMillisecond());
                    break;
                    
                default:
                    break;
            }
        }
        
        
        bool ControlNode::isRamping() const
        {
            return mLinearRamper.isRamping() || mExponentialRamper.isRamping();
        }
        
        
        void ControlNode::stop()
        {
            mLinearRamper.stop();
            mExponentialRamper.stop();
        }
        
        
        void ControlNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            
            if (mTranslator)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mLinearRamper.step();
                    mExponentialRamper.step();
                    outputBuffer[i] = mTranslator->translate(mValue);
                }
            }
            else {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    mLinearRamper.step();
                    mExponentialRamper.step();
                    outputBuffer[i] = mValue;
                }
            }
        }
        
    }
    
}
