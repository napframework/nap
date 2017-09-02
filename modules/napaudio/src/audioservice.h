#pragma once

// Std includes
#include <mutex>

// Nap includes
#include <nap/service.h>
#include <nap/threading.h>

// Audio includes
#include "audiotypes.h"

namespace nap {
    
    namespace audio {
    
        // Forward declarations
        class AudioNode;
        class AudioTrigger;
        
        /*
         * Services that takes care of the processing of the audio nodes for an audio device
         */
        class NAPAPI AudioService : public Service {
            RTTI_ENABLE(Service)
            
            friend class AudioNode;
            friend class AudioTrigger;
            friend class AudioOutputNode;
            friend class AudioInputNode;
            
        public:
            using OutputMapping = std::vector<std::vector<SampleBufferPtr>>;
            
        public:
            AudioService() = default;
            
            /*
             * This function is called on every audio callback.
             * It's task is to fill @outputBuffer with an array of @framesPerBuffer samples for every channel.
             */
            void process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);
            
            // Returns the number of active input channels on the audio device
            int getInputChannelCount() const { return mInputChannelCount; }
            // Returns the number of active output channels on the audio device
            int getOutputChannelCount() const { return mOutputChannelCount; }
            // Returns the sample rate the audio device is currently running on
            float getSampleRate() const { return mSampleRate; }
            /* 
             * Returns the size of the buffer the node system is running on.
             * Beware: this can be smaller than the buffersize the audio device is running on.
             * The latter is specified by the framesPerBuffer parameter in the process() function.
             */
            int getInternalBufferSize() const { return mInternalBufferSize; }
            
            // Returns the absolute time in samples
            const DiscreteTimeValue& getSampleTime() const { return mSampleTime; }
            
            void setInputChannelCount(int inputChannelCount);
            void setOutputChannelCount(int outputChannelCount);
            void setSampleRate(float sampleRate);
            void setInternalBufferSize(int size);
            
            // Enqueue a task to be executed within the process() method for thread safety
            void execute(TaskQueue::Task task) { mAudioCallbackTaskQueue.enqueue(task); }
            
        private:
            void registerNode(AudioNode& node);
            void unregisterNode(AudioNode& node) { mAudioNodes.erase(&node); }
            void registerTrigger(AudioTrigger& trigger) { mAudioTriggers.emplace(&trigger); }
            void unregisterTrigger(AudioTrigger& trigger) { mAudioTriggers.erase(&trigger); }
            
        private:
            void addOutputBuffer(SampleBufferPtr buffer, int channel);
            const float& getInputSample(int channel, int index) const { return mInputBuffer[channel][mInternalBufferOffset + index]; }
            
            int mInputDevice = 0;
            int mOutputDevice = 0;
            int mInputChannelCount = 1;
            int mOutputChannelCount = 2;
            float mSampleRate = 44100.0;
            int mInternalBufferSize = 64;
            
            DiscreteTimeValue mSampleTime = 0;
            unsigned int mInternalBufferOffset = 0;
            
            // for each output channel a vector of buffers that need to be played back on the corresponding channel
            OutputMapping mOutputMapping;
            
            float** mInputBuffer = nullptr;
            
            std::set<AudioNode*> mAudioNodes;
            std::set<AudioTrigger*> mAudioTriggers;
            
            std::mutex mAudioCallbackMutex;
            nap::TaskQueue mAudioCallbackTaskQueue;
        };
        
    }
}
