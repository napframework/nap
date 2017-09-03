// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audiodevice.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioInterface)
    RTTI_PROPERTY("UseDefaultDevice", &nap::audio::AudioInterface::mUseDefaultDevice, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("InputDevice", &nap::audio::AudioInterface::mInputDevice, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("OutputDevice", &nap::audio::AudioInterface::mOutputDevice, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("InputChannelCount", &nap::audio::AudioInterface::mInputChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("OutputChannelCount", &nap::audio::AudioInterface::mOutputChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("SampleRate", &nap::audio::AudioInterface::mSampleRate, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("BufferSize", &nap::audio::AudioInterface::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
        static int audioCallback( const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData )
        {
            float** out = (float**)outputBuffer;
            float** in = (float**)inputBuffer;
            
            
            AudioNodeManager* nodeManager = reinterpret_cast<AudioNodeManager*>(userData);
            nodeManager->process(in, out, framesPerBuffer);
            
            return 0;
        }
        
        
        AudioInterface::AudioInterface()
        {
            mInitialized = AudioDeviceManager::init();
        }
        
        
        
        AudioInterface::~AudioInterface()
        {
            stop();
            
            if (mInitialized)
                AudioDeviceManager::terminate();
        }
        
        
        bool AudioInterface::init(utility::ErrorState& errorState)
        {
            start();
            return true;
        }
        
        
        void AudioInterface::start()
        {
            if (mUseDefaultDevice)
            {
                startDefaultDevice();
                return;
            }
            
            PaStreamParameters inputParameters;
            inputParameters.device = mInputDevice;
            inputParameters.channelCount = mInputChannelCount;
            inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            inputParameters.suggestedLatency = 0;
            inputParameters.hostApiSpecificStreamInfo = nullptr;
            
            PaStreamParameters outputParameters;
            outputParameters.device = mOutputDevice;
            outputParameters.channelCount = mOutputChannelCount;
            outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            outputParameters.suggestedLatency = 0;
            outputParameters.hostApiSpecificStreamInfo = nullptr;
            
            auto error = Pa_OpenStream(&mStream, &inputParameters, &outputParameters, mSampleRate, mBufferSize, paNoFlag, audioCallback, &mNodeManager);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            if (error == paNoError)
            {
                PaDeviceInfo inputInfo = AudioDeviceManager::getDeviceInfo(mInputDevice);
                PaDeviceInfo outputInfo = AudioDeviceManager::getDeviceInfo(mOutputDevice);
                Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", inputInfo.name, outputInfo.name, mInputChannelCount, mOutputChannelCount, mSampleRate, mBufferSize);
            }
        }
        
        
        void AudioInterface::startDefaultDevice()
        {
            auto error = Pa_OpenDefaultStream(&mStream, mInputChannelCount, mOutputChannelCount, paFloat32 | paNonInterleaved, mSampleRate, mBufferSize, audioCallback, &mNodeManager);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            if (error == paNoError)
                Logger::info("Portaudio default stream started: %i inputs, %i outputs, samplerate %i, buffersize %i", mInputChannelCount, mOutputChannelCount, int(mSampleRate), mBufferSize);
        }
                
        
        void AudioInterface::stop()
        {
            if (mStream && Pa_IsStreamActive(mStream) == 1)
            {
                Pa_StopStream(mStream);
                Pa_CloseStream(mStream);
                mStream = nullptr;
            }
            Logger::info("Portaudio stopped");
        }
        
        
        bool AudioInterface::isActive()
        {
            return (Pa_IsStreamActive(mStream) == 1);
        }
        
        
        bool AudioDeviceManager::init()
        {
            auto error = Pa_Initialize();
            if (error != paNoError)
            {
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            return true;
        }
        
        
        void AudioDeviceManager::terminate()
        {
            auto error = Pa_Terminate();
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
        }
        
        
        unsigned int AudioDeviceManager::getDeviceCount()
        {
            return Pa_GetDeviceCount();
        }
        
        
        const PaDeviceInfo& AudioDeviceManager::getDeviceInfo(unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(deviceIndex);
        }
        
        
        std::vector<const PaDeviceInfo*> AudioDeviceManager::getDevices()
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                result.emplace_back(&getDeviceInfo(i));
            }
            return result;
        }
        
        
        void AudioDeviceManager::printDevices()
        {
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                const PaDeviceInfo& info = *Pa_GetDeviceInfo(i);
                std::cout << i << " " << info.name << " " << info.maxInputChannels << " inputs " << info.maxOutputChannels << " outputs" << std::endl;
            }
        }
        
    }
}
