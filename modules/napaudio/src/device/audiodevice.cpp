#include "audiointerface.h"

// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audioservice.h"

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
        
        
        /*
         * The audio callback will be called by portaudio to process a buffer of audio input/output
         */
        static int audioCallback( const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData )
        {
            float** out = (float**)outputBuffer;
            float** in = (float**)inputBuffer;
            
            
            NodeManager* nodeManager = reinterpret_cast<NodeManager*>(userData);
            nodeManager->process(in, out, framesPerBuffer);
            
            return 0;
        }
        
        
        AudioInterface::AudioInterface(AudioDeviceService& service) : mService(&service)
        {
            
        }
        
        
        /*
         * The destructor stops the audio stream if it is active.
         */
        AudioInterface::~AudioInterface()
        {
            stop();
        }
        
        
        /*
         * Starts the audio stream
         */
        bool AudioInterface::init(utility::ErrorState& errorState)
        {
            return start(errorState);
        }
        
        
        bool AudioInterface::start(utility::ErrorState& errorState)
        {
            if (isActive())
                return true;
            
            if (mUseDefaultDevice)
                return startDefaultDevice(errorState);
            
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
                errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
            {
                errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            PaDeviceInfo inputInfo = mService->getDeviceInfo(mInputDevice);
            PaDeviceInfo outputInfo = mService->getDeviceInfo(mOutputDevice);
            Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", inputInfo.name, outputInfo.name, mInputChannelCount, mOutputChannelCount, mSampleRate, mBufferSize);
            
            return true;
        }
        
        
        bool AudioInterface::startDefaultDevice(utility::ErrorState& errorState)
        {
            auto error = Pa_OpenDefaultStream(&mStream, mInputChannelCount, mOutputChannelCount, paFloat32 | paNonInterleaved, mSampleRate, mBufferSize, audioCallback, &mNodeManager);
            if (error != paNoError)
            {
                errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
                return false;
            }
            
            mNodeManager.setInputChannelCount(mInputChannelCount);
            mNodeManager.setOutputChannelCount(mOutputChannelCount);
            mNodeManager.setSampleRate(mSampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
            {
                errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
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
