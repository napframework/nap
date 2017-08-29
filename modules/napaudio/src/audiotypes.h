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
        using NAPAPI SampleValue = float;
        
        using NAPAPI SampleBuffer = std::vector<SampleValue>;

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
        
        /*
         * Value of control parameter
         * Change this to double to build with double precision sample calculation
         */
        using NAPAPI ControllerValue = float;
        
        /*
         * Time value in milliseconds
         */
        using NAPAPI TimeValue = float;
        
        /*
         * Time value in samples
         */
        using NAPAPI DiscreteTimeValue = long;
        
        
        constexpr NAPAPI ControllerValue pi = 3.1415926535897932384626433832795028841971693993751;
        
    }
        
}
