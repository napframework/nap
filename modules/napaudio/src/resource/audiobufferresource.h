#pragma once

// nap includes
#include <rtti/rttiobject.h>

// audio includes
#include <utility/audiotypes.h>

namespace nap {
    
    namespace audio {
        
        /**
         * A base class for a buffer of multichannel audio data in memory
         * Base class for AudioFileResource and AudioBufferResource
         */
        class AudioBufferResourceBase : public rtti::RTTIObject
        {
            RTTI_ENABLE(rtti::RTTIObject)
        public:
            AudioBufferResourceBase() = default;
            
            /**
             * @return: sample rate at which the audio material in the buffer was sampled.
             */
            float getSampleRate() const { return mSampleRate; }
            
            /**
             * @return: size of the buffer in samples
             */
            DiscreteTimeValue getSize() const { return mBuffer.getSize(); }
            
            /**
             * @return: number of channels in the buffer
             */
            unsigned int getChannelCount() const { return mBuffer.getChannelCount(); }
            
            /**
             * @return: access the actual data in the buffer
             */
            MultiSampleBuffer& getBuffer() { return mBuffer; }
            
        protected:
            /**
             * Sets the sample rate at which the audio material in the buffer was sampled.
             */
            void setSampleRate(float sampleRate) { mSampleRate = sampleRate; }            

        private:
            float mSampleRate = 0;
            MultiSampleBuffer mBuffer;
            
        };
        
        
    }
    
}
