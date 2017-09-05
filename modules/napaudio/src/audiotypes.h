#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>

namespace nap {
    
    namespace audio {
        
        // Note: in this files typedefs are being used instead of a using statement
        // This is because the using statement cannot be used in combination with the NAPAPI macro
        
        
        /**
         * Value of a single audio sample
         * Change this to double to build with double precision sample calculation
         */
        using SampleValue = float;
        
        
        /**
         * A buffer of samples
         */
        using SampleBuffer = std::vector<SampleValue>;
        using SampleBufferPtr = SampleBuffer*;

        
        /**
         * A collection of sample buffers, one for each channel to represent multichannel audio.
         */
        class NAPAPI MultiSampleBuffer {
        public:
            MultiSampleBuffer() = default;
            
            /**
             * @param channelCount: number of channels in this buffer
             * @param size: size of the buffer in samples
             */
            MultiSampleBuffer(size_t channelCount, size_t size) { resize(channelCount, size); }
            
            /**
             * Used to access the samples in the buffer
             * example: myBuffer[channelNumber][sampleIndex]
             */
            SampleBuffer& operator[](size_t index) { return channels[index]; }
            
            /**
             * @return: number of channels in the buffer
             */
            size_t getChannelCount() const { return channels.size(); }
            
            /**
             * @return: the size of the buffer in samples
             */
            size_t getSize() const { return channels.empty() ? 0 : channels.front().size();  }
            
            /**
             * Resize the buffer
             * @param channelCount: new number of channels
             * @param size: new size in samples
             */
            void resize(size_t channelCount, size_t size)
            {
                channels.resize(channelCount);
                for (auto& channel : channels)
                    channel.resize(size);
            }
            
            std::vector<SampleBuffer> channels;
        };
        typedef NAPAPI MultiSampleBuffer* MultiSampleBufferPtr;
        
        
        /**
         * Value of control parameter
         * Change this to double to build with double precision sample calculation
         */
        using ControllerValue = float;
        
        
        /**
         * Time value in milliseconds
         */
        using TimeValue = float;
        
        
        /**
         * Time value in samples
         */
        using DiscreteTimeValue = long;
        
        
        /**
         * Pi
         */
        constexpr NAPAPI ControllerValue pi = 3.1415926535897932384626433832795028841971693993751;
        
    }
        
}
