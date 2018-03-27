#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>

namespace nap
{
    
    namespace audio
    {
        
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
        class NAPAPI MultiSampleBuffer
        {
        public:
            MultiSampleBuffer() = default;
            
            /**
             * @param channelCount: number of channels in this buffer
             * @param size: size of the buffer in samples
             */
            MultiSampleBuffer(std::size_t channelCount, std::size_t size)
            {
                resize(channelCount, size);
            }
            
            /**
             * Used to access the samples in the buffer
             * example: myBuffer[channelNumber][sampleIndex]
             */
            SampleBuffer& operator[](std::size_t index) { return *channels[index]; }
            
            /**
             * @return: number of channels in the buffer
             */
            std::size_t getChannelCount() const { return channels.size(); }
            
            /**
             * @return: the size of the buffer in samples
             */
            std::size_t getSize() const { return channels.empty() ? 0 : channels.front()->size();  }
            
            /**
             * @return: channel as a shared pointer that can be thread-safely shared with audio nodes
             */
            std::shared_ptr<SampleBuffer>& getChannelPtr(size_t channel) { return channels[channel]; }
            
            /**
             * Resize the buffer
             * @param channelCount: new number of channels
             * @param size: new size in samples
             */
            void resize(std::size_t channelCount, std::size_t size)
            {
                auto oldChannelCount = channels.size();
                channels.resize(channelCount);
                for (auto channel = 0; channel < channelCount; ++channel)
                {
                    if (channel >= oldChannelCount)
                        channels[channel] = std::make_shared<SampleBuffer>();
                    channels[channel]->resize(size);
                }
            }
            
            /**
             * Clear the content of the buffer.
             */
            void clear()
            {
                channels.clear();
            }
            
        private:
            std::vector<std::shared_ptr<SampleBuffer>> channels;
        };
        
        
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
        
    }
        
}
