#pragma once

#include <utility/audiotypes.h>

namespace nap
{
    namespace audio
    {
        class Delay
        {
        public:
            Delay(unsigned int bufferSize);
            ~Delay();

            void write(SampleValue sample);
            SampleValue read(unsigned int time);
            SampleValue readInterpolating(float sampleTime);
            void clear();
            
            unsigned int getMaxDelay() { return bufferSize; }
            
            inline SampleValue operator[](unsigned int index)
            {
                return read(index);
            }
            
        private:
            
            SampleValue* buffer = nullptr;
            unsigned int bufferSize = 0;
            unsigned int writeIndex = 0;
        };
                
    }
}
