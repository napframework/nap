#include "delay.h"

#include <utility/audiofunctions.h>

namespace nap {
    
    namespace audio {
        
        Delay::Delay(unsigned int bufferSize) :
            bufferSize(bufferSize)
        {
            buffer = new SampleValue[bufferSize];
            writeIndex = 0;
            
            clear();
        }
        
        Delay::~Delay()
        {
            delete[] buffer;
        }


        void Delay::write(SampleValue sample)
        {
            buffer[writeIndex++] = sample;
            writeIndex = wrap(writeIndex, bufferSize);
        }


        SampleValue Delay::readInterpolating(float time)
        {
            // Update the read index
            SampleValue readIndex = writeIndex - time - 1;
            while (readIndex < 0) readIndex += bufferSize;
            
            unsigned int floorReadIndex = (unsigned int)readIndex;
            unsigned int integerIndex = wrap(floorReadIndex, bufferSize);
            unsigned int nextIntegerIndex = wrap(integerIndex+1, bufferSize);
            
            SampleValue frac = readIndex - floorReadIndex;
            
            return lerp(buffer[integerIndex], buffer[nextIntegerIndex], frac);
        }
        
        
        SampleValue Delay::read(unsigned int time)
        {
            // Update the read index
            int readIndex = writeIndex - time - 1;
            while (readIndex < 0) readIndex += bufferSize;            
            
            unsigned int integerIndex = wrap(readIndex, bufferSize);
            
            return buffer[integerIndex];
        }
        
        
        void Delay::clear()
        {
//            memset(buffer, 0, bufferSize * sizeof(SampleValue));
            
            for (unsigned int i = 0; i < bufferSize; i++)
            {
                buffer[i] = 0.0;
            }            
        }

        
    }
}
