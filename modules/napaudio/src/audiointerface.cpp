#include "audiointerface.h"

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
    RTTI_PROPERTY("AllowFailure", &nap::audio::AudioInterface::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
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
        
        
        AudioInterface::~AudioInterface()
        {
            stop();
            
            if (mInitialized)
            {
                auto error = Pa_Terminate();
                if (error != paNoError)
                    Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            }
        }
        
        
        bool AudioInterface::init(utility::ErrorState& errorState)
        {
            auto error = Pa_Initialize();
            if (error != paNoError)
            {
                mInitialized = false;
                std::string message = "Portaudio error: " + std::string(Pa_GetErrorText(error));
                nap::Logger::warn(message);
                return errorState.check(mAllowFailure, message);
            }
            mInitialized = true;
            
            if (!start())
                return errorState.check(mAllowFailure, "Portaudio error: failed to start audio stream");
            
            return true;
        }
        
        
        bool AudioInterface::start()
        {
            if (isActive())
                return;
            
            if (mUseDefaultDevice)
                return startDefaultDevice();
            
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
            {
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
            {
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            PaDeviceInfo inputInfo = AudioDeviceManager::getDeviceInfo(mInputDevice);
            PaDeviceInfo outputInfo = AudioDeviceManager::getDeviceInfo(mOutputDevice);
            Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", inputInfo.name, outputInfo.name, mInputChannelCount, mOutputChannelCount, mSampleRate, mBufferSize);
            return true;
        }
        
        
        bool AudioInterface::startDefaultDevice()
        {
            auto error = Pa_OpenDefaultStream(&mStream, mInputChannelCount, mOutputChannelCount, paFloat32 | paNonInterleaved, mSampleRate, mBufferSize, audioCallback, &mNodeManager);
            if (error != paNoError)
            {
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
            {
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            Logger::info("Portaudio default stream started: %i inputs, %i outputs, samplerate %i, buffersize %i", mInputChannelCount, mOutputChannelCount, int(mSampleRate), mBufferSize);
            return true;
        }
        
        
        void AudioInterface::stop()
        {
            if (isActive())
            {
                Pa_StopStream(mStream);
                Pa_CloseStream(mStream);
                mStream = nullptr;
                Logger::info("Portaudio stopped");
            }
        }
        
        
        bool AudioInterface::isActive()
        {
            return (mStream && Pa_IsStreamActive(mStream) == 1);
        }
        
        
     }
}
