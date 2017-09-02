#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>

namespace nap {
    
    namespace audio {
    
        /*
         * Value of a single audio sample
         * Change this to double to build with double precision sample calculation
         */
        typedef NAPAPI float SampleValue;
        
        typedef NAPAPI std::vector<SampleValue> SampleBuffer;
        typedef NAPAPI SampleBuffer* SampleBufferPtr;

        class NAPAPI MultiSampleBuffer {
        public:
            MultiSampleBuffer() = default;
            MultiSampleBuffer(size_t channelCount, size_t size) { resize(channelCount, size); }
            
            SampleBuffer& operator[](size_t index) { return channels[index]; }
            
            size_t getChannelCount() const { return channels.size(); }
            size_t getSize() const { return channels.empty() ? 0 : channels.front().size();  }
            
            void resize(size_t channelCount, size_t size)
            {
                channels.resize(channelCount);
                for (auto& channel : channels)
                    channel.resize(size);
            }
            
            std::vector<SampleBuffer> channels;
        };
        typedef NAPAPI MultiSampleBuffer* MultiSampleBufferPtr;
        
        /*
         * Value of control parameter
         * Change this to double to build with double precision sample calculation
         */
        typedef NAPAPI float ControllerValue;
        
        /*
         * Time value in milliseconds
         */
		typedef NAPAPI float TimeValue;
        
        /*
         * Time value in samples
         */
		typedef NAPAPI long DiscreteTimeValue;
        
        
        constexpr NAPAPI ControllerValue pi = 3.1415926535897932384626433832795028841971693993751;
        
    }
        
}
