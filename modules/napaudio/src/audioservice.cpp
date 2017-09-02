#include "audioservice.h"
#include "audionode.h"

#include <nap/logger.h>
#include <nap/core.h>

namespace nap {
    
    namespace audio {
    
        void AudioService::process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
        {
            // process tasks that are enqueued from outside the audio thread
            mAudioCallbackTaskQueue.process();
            
            // clean the output buffers
            for (auto channel = 0; channel < mOutputChannelCount; ++channel)
                memset(outputBuffer[channel], 0, sizeof(float) * framesPerBuffer);
            
            mInputBuffer = inputBuffer;
            
            mInternalBufferOffset = 0;
            while (mInternalBufferOffset < framesPerBuffer)
            {
                for (auto& channelMapping : mOutputMapping)
                    channelMapping.clear();
                for (auto& trigger : mAudioTriggers)
                    trigger->trigger();
                
                for (auto channel  = 0; channel < mOutputChannelCount; ++channel)
                {
                    for (auto& output : mOutputMapping[channel])
                        for (auto j = 0; j < mInternalBufferSize; ++j)
                            outputBuffer[channel][mInternalBufferOffset + j] += (*output)[j];
                }
                
                mInternalBufferOffset += mInternalBufferSize;
                mSampleTime += mInternalBufferSize;
            }
        }
        
        
        void AudioService::setInputChannelCount(int inputChannelCount)
        {
            mInputChannelCount = inputChannelCount;
        }
        
        
        
        void AudioService::setOutputChannelCount(int outputChannelCount)
        {
            mOutputChannelCount = outputChannelCount;
            mOutputMapping.clear();
            mOutputMapping.resize(mOutputChannelCount);
        }
        
        
        void AudioService::setInternalBufferSize(int size)
        {
            mInternalBufferSize = size;
            for (auto& node : mAudioNodes)
                node->setBufferSize(size);
        }
        
        
        void AudioService::setSampleRate(float sampleRate)
        {
            mSampleRate = sampleRate;
            for (auto& node : mAudioNodes)
                node->setSampleRate(sampleRate);
        }
        
        
        void AudioService::registerNode(AudioNode& node)
        {
            mAudioNodes.emplace(&node);
            node.setSampleRate(mSampleRate);
            node.setBufferSize(mInternalBufferSize);
        }
        
        
        void AudioService::addOutputBuffer(SampleBufferPtr buffer, int channel)
        {
            assert(channel < mOutputMapping.size());
            mOutputMapping[channel].emplace_back(buffer);
        }
        
    }
    
}

RTTI_DEFINE(nap::audio::AudioService)

