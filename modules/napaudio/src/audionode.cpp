#include "audionode.h"
#include "audioservice.h"


namespace nap {
    
    namespace audio {
        
        
        SampleBufferPtr AudioInput::pull()
        {
            if (mConnection)
                return mConnection->pull();
            else
                return nullptr;
        }
        
        AudioOutput::AudioOutput(AudioNode* node) : mNode(node)
        {
            mNode->mOutputs.emplace(this);
            setBufferSize(mNode->getBufferSize());
        }
        
        
        AudioOutput::~AudioOutput()
        {
            mNode->mOutputs.erase(this);
        }
        
        
        SampleBufferPtr AudioOutput::pull()
        {
            if (mLastCalculatedSample < mNode->getSampleTime())
            {
                mCalculateFunction(mBuffer);
                mLastCalculatedSample += mBuffer.size();
            }
            return &mBuffer;
        }
        
        
        void AudioOutput::setBufferSize(int bufferSize)
        {
            mBuffer.resize(bufferSize);
        }
        
        
        int AudioNode::getBufferSize() const
        {
            return mAudioService->getInternalBufferSize();
        }
        
        
        float AudioNode::getSampleRate() const
        {
            return mAudioService->getSampleRate();
        }
        
        
        DiscreteTimeValue AudioNode::getSampleTime() const
        {
            return mAudioService->getSampleTime();
        }
        
        
        AudioNode::AudioNode(AudioService& service)
        {
            mAudioService = &service;
            service.registerNode(*this);
        }
        
        
        AudioNode::~AudioNode()
        {
            mAudioService->unregisterNode(*this);
        }
        
        
        void AudioNode::setBufferSize(int bufferSize)
        {
            for (auto& output : mOutputs)
                output->setBufferSize(bufferSize);
            
            bufferSizeChanged(bufferSize);
        }
        
        
        AudioTrigger::AudioTrigger(AudioService& service) : AudioNode(service)
        {
            service.registerTrigger(*this);
        }
        
        
        AudioTrigger::~AudioTrigger()
        {
            mAudioService->unregisterTrigger(*this);
        }
        
        
        
        void AudioOutputNode::trigger()
        {
            SampleBufferPtr buffer = audioInput.pull();
            if (buffer)
                mAudioService->addOutputBuffer(buffer, outputChannel);
        }
        
        
        void AudioInputNode::fill(SampleBuffer& buffer)
        {
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = mAudioService->getInputSample(inputChannel, i);
        }

    }
        
}

