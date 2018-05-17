#include "bufferplayernode.h"

namespace nap
{
    
    namespace audio
    {
        
        void BufferPlayerNode::play(SafePtr<MultiSampleBuffer> buffer, int channel, DiscreteTimeValue position, ControllerValue speed)
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
            
            auto playing = mPlaying.load();
            auto channel = mChannel.load();
            auto position = mPosition.load();
            auto speed = mSpeed.load();
                        
            // If we're not playing, fill the buffer with 0's and bail out.
            if (!playing || mBuffer == nullptr || channel >= mBuffer->getChannelCount())
            {
                std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
                return;
            }
            
            DiscreteTimeValue flooredPosition;
            SampleValue lastValue, newValue, fractionalPart;
            SampleBuffer& channelBuffer = (*mBuffer)[channel];
            
            // For each sample
            for (auto i = 0; i < outputBuffer.size(); i++)
            {
                // Have we reached the destination?
                if (position + 1 >= channelBuffer.size())
                {
                    outputBuffer[i] = 0;
                    if (playing)
                        playing = false;
                }
                else {
                    flooredPosition = DiscreteTimeValue(position);
                    lastValue = channelBuffer[flooredPosition];
                    newValue = channelBuffer[flooredPosition + 1];
                    
                    fractionalPart = position - flooredPosition;
                    
                    outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));
                    
                    position += speed;
                }
            }
            
            mPosition.store(position);
            mPlaying.store(playing);
        }
        
    }
    
}
