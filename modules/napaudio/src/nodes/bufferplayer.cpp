#include "bufferplayer.h"

// Std includes
#include <cstring>

namespace nap {
    
    namespace audio {
        
        void BufferPlayer::play(SampleBuffer& buffer, DiscreteTimeValue position, ControllerValue speed)
        {
            mPlaying = true;
            mBuffer = &buffer;
            mPosition = position;
            mSpeed = speed;
        }
        
        
        void BufferPlayer::stop()
        {
            mPlaying = false;
        }
        
        
        void BufferPlayer::calculate(SampleBuffer& outputBuffer)
        {
            DiscreteTimeValue flooredPosition;
            SampleValue lastValue, newValue, fractionalPart;
            
            // If we're not playing, fill the buffer with 0's and bail out.
            if (!mPlaying || mBuffer == nullptr)
            {
                std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
                return;
            }
            
            // For each sample
            for (auto i = 0; i < outputBuffer.size(); i++)
            {
                // Have we reached the destination?
                if (mPosition + 1 >= mBuffer->size())
                {
                    outputBuffer[i] = 0;
                    if (mPlaying)
                        mPlaying = false;
                }
                else {
                    flooredPosition = DiscreteTimeValue(mPosition);
                    lastValue = (*mBuffer)[flooredPosition];
                    newValue = (*mBuffer)[flooredPosition + 1];
                    
                    fractionalPart = mPosition - flooredPosition;
                    
                    outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));
                    
                    mPosition += mSpeed;
                }
            }
        }
        
    }
    
}
