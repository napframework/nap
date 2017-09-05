#include "audionode.h"
#include "audionodemanager.h"


namespace nap {
    
    namespace audio {
        
        
        AudioInput::~AudioInput()
        {
            disconnect();
        }
        
        
        SampleBufferPtr AudioInput::pull()
        {
            if (mConnection)
                return mConnection->pull();
            else
                return nullptr;
        }
        
        
        void AudioInput::connect(AudioOutput& connection)
        {
            // disconnect both input and output
            disconnect();
            connection.disconnect();
            
            // make the input and output point to one another
            mConnection = &connection;
            connection.mConnection = this;
        }
        
        
        void AudioInput::disconnect()
        {
            if (mConnection)
            {
                mConnection->mConnection = nullptr;
                mConnection = nullptr;
            }
        }

        
        AudioOutput::~AudioOutput()
        {
            mNode->mOutputs.erase(this);
            if (mConnection)
                disconnect();
        }
        
        
        void AudioOutput::disconnect()
        {
            if (mConnection)
                mConnection->disconnect();
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
            return mAudioNodeManager->getInternalBufferSize();
        }
        
        
        float AudioNode::getSampleRate() const
        {
            return mAudioNodeManager->getSampleRate();
        }
        
        
        DiscreteTimeValue AudioNode::getSampleTime() const
        {
            return mAudioNodeManager->getSampleTime();
        }
        
        
        AudioNode::AudioNode(AudioNodeManager& service)
        {
            mAudioNodeManager = &service;
            service.registerNode(*this);
        }
        
        
        AudioNode::~AudioNode()
        {
            mAudioNodeManager->unregisterNode(*this);
        }
        
        
        void AudioNode::setBufferSize(int bufferSize)
        {
            for (auto& output : mOutputs)
                output->setBufferSize(bufferSize);
            
            bufferSizeChanged(bufferSize);
        }
        
        
        AudioTrigger::AudioTrigger(AudioNodeManager& service) : AudioNode(service)
        {
            service.registerTrigger(*this);
        }
        
        
        AudioTrigger::~AudioTrigger()
        {
            mAudioNodeManager->unregisterTrigger(*this);
        }
        
        
        
        void AudioOutputNode::trigger()
        {
            SampleBufferPtr buffer = audioInput.pull();
            if (buffer)
                mAudioNodeManager->provideOutputBufferForChannel(buffer, mOutputChannel);
        }
        
        
        void AudioInputNode::fill(SampleBuffer& buffer)
        {
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = mAudioNodeManager->getInputSample(mInputChannel, i);
        }

    }
        
}

