#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>

namespace nap {
    
    namespace audio {
    
        /**
         * Value of a single audio sample
         * Change this to double to build with double precision sample calculation
         */
//        typedef NAPAPI float SampleValue;
        NAPAPI using SampleValue = float;
        
        
        /**
         * A buffer of samples
         */
        typedef NAPAPI std::vector<SampleValue> SampleBuffer;
        typedef NAPAPI SampleBuffer* SampleBufferPtr;

        
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
        typedef NAPAPI float ControllerValue;
        
        
        /**
         * Time value in milliseconds
         */
		typedef NAPAPI float TimeValue;
        
        
        /**
         * Time value in samples
         */
		typedef NAPAPI long DiscreteTimeValue;
        
        
        /**
         * Pi
         */
        constexpr NAPAPI ControllerValue pi = 3.1415926535897932384626433832795028841971693993751;
        
    }
        
}
