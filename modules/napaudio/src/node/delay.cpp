#include "delay.h"

#include <utility/audiofunctions.h>

namespace nap
{
    
    namespace audio
    {
        
        Delay::Delay(unsigned int bufferSize) :
            mBufferSize(bufferSize)
        {
            mBuffer = new SampleValue[bufferSize];
            mWriteIndex = 0;
            
            clear();
        }
        
        Delay::~Delay()
        {
            delete[] mBuffer;
        }


        void Delay::write(SampleValue sample)
        {
            mBuffer[mWriteIndex++] = sample;
            mWriteIndex = wrap(mWriteIndex, mBufferSize);
        }


        SampleValue Delay::readInterpolating(float time)
        {
            // Update the read index
            SampleValue readIndex = mWriteIndex - time - 1;
            while (readIndex < 0) readIndex += mBufferSize;
            
            unsigned int floorReadIndex = (unsigned int)readIndex;
            unsigned int integerIndex = wrap(floorReadIndex, mBufferSize);
            unsigned int nextIntegerIndex = wrap(integerIndex+1, mBufferSize);
            
            SampleValue frac = readIndex - floorReadIndex;
            
            return lerp(mBuffer[integerIndex], mBuffer[nextIntegerIndex], frac);
        }
        
        
        SampleValue Delay::read(unsigned int time)
        {
            // Update the read index
            int readIndex = mWriteIndex - time - 1;
            while (readIndex < 0) readIndex += mBufferSize;
            
            unsigned int integerIndex = wrap(readIndex, mBufferSize);
            
            return mBuffer[integerIndex];
        }
        
        
        void Delay::clear()
        {
            for (unsigned int i = 0; i < mBufferSize; i++)
            {
                mBuffer[i] = 0.0;
            }            
        }

        
    }
}
