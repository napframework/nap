#include "bufferplayernode.h"

namespace nap
{
    
    namespace audio
    {
        
        void BufferPlayerNode::play(utility::SafePtr<MultiSampleBuffer> buffer, int channel, DiscreteTimeValue position, ControllerValue speed)
        {
            mPlaying = true;
            mBuffer = buffer;
            mChannel = channel;
            mPosition = position;
            mSpeed = speed;
        }
        
        
        void BufferPlayerNode::stop()
        {
            mPlaying = false;
        }
        
        
        void BufferPlayerNode::setChannel(int channel)
        {
            mChannel = channel;
        }
        
        
        void BufferPlayerNode::setSpeed(ControllerValue speed)
        {
            mSpeed = speed;
        }
        
        
        void BufferPlayerNode::setPosition(DiscreteTimeValue position)
        {
            mPosition = position;
        }
        
        
        void BufferPlayerNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            
            // If we're not playing, fill the buffer with 0's and bail out.
            if (!mPlaying || mBuffer == nullptr || mChannel >= mBuffer->getChannelCount())
            {
                std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
                return;
            }
            
            DiscreteTimeValue flooredPosition;
            SampleValue lastValue, newValue, fractionalPart;
            SampleBuffer& channelBuffer = (*mBuffer)[mChannel];
            
            // For each sample
            for (auto i = 0; i < outputBuffer.size(); i++)
            {
                // Have we reached the destination?
                if (mPosition + 1 >= channelBuffer.size())
                {
                    outputBuffer[i] = 0;
                    if (mPlaying)
                        mPlaying = false;
                }
                else {
                    flooredPosition = DiscreteTimeValue(mPosition);
                    lastValue = channelBuffer[flooredPosition];
                    newValue = channelBuffer[flooredPosition + 1];
                    
                    fractionalPart = mPosition - flooredPosition;
                    
                    outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));
                    
                    mPosition += mSpeed;
                }
            }
        }
        
    }
    
}
