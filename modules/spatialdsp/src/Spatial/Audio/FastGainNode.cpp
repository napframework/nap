#include "FastGainNode.h"

// Audio includes
#include <audio/core/audionodemanager.h>


// RTTI
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FastGainNode)
    RTTI_PROPERTY("audioInput", &nap::audio::FastGainNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::FastGainNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setGain", &nap::audio::FastGainNode::setGain)
    RTTI_FUNCTION("getGain", &nap::audio::FastGainNode::getGain)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {


        FastGainNode::FastGainNode(NodeManager& manager, ControllerValue gain, TimeValue rampTime) : Node(manager), mGain(gain, rampTime * manager.getSamplesPerMillisecond())
        {
        }

        
        void FastGainNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto& inputBuffer = *audioInput.pull();

            mGain.update();

            if (mGain.isRamping() == false)
            {
                if (mGain.getValue() == 0.f)
                {
                	if (mOutputIsZero == false)
                	{
                		mOutputIsZero = true;
						for (auto i = 0; i < outputBuffer.size(); ++i)
							outputBuffer[i] = 0.f;
					}
				}
				else
				{
					mOutputIsZero = false;
					
					if (mGain.getValue() == 1.f)
						for (auto i = 0; i < outputBuffer.size(); ++i)
							outputBuffer[i] = inputBuffer[i];
					else
						for (auto i = 0; i < outputBuffer.size(); ++i)
							outputBuffer[i] = inputBuffer[i] * mGain.getValue();
				}
            }
            else
            {
            	mOutputIsZero = false;
				
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] = inputBuffer[i] * mGain.getNextValue();
			}
        }


        void FastGainNode::setRampTime(TimeValue rampTime)
        {
            mGain.setStepCount(rampTime * getNodeManager().getSamplesPerMillisecond());
        }
        
        

    }
    
}
