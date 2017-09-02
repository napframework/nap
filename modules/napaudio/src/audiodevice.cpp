// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audiodevice.h"
#include "audionodemanager.h"

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
        
        
        AudioDeviceManager::AudioDeviceManager(AudioNodeManager& nodeManager) : mNodeManager(nodeManager)
        {
            auto error = Pa_Initialize();
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));            
        }
        
        
        AudioDeviceManager::~AudioDeviceManager()
        {
            stop();
            
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

        
        
        void AudioDeviceManager::start(int inputDevice, int outputDevice, int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize)
        {
            PaStreamParameters inputParameters;
            inputParameters.device = inputDevice;
            inputParameters.channelCount = inputChannelCount;
            inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            inputParameters.suggestedLatency = 0;
            inputParameters.hostApiSpecificStreamInfo = nullptr;
            
            PaStreamParameters outputParameters;
            outputParameters.device = outputDevice;
            outputParameters.channelCount = outputChannelCount;
            outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            outputParameters.suggestedLatency = 0;
            outputParameters.hostApiSpecificStreamInfo = nullptr;
            
            auto error = Pa_OpenStream(&mStream, &inputParameters, &outputParameters, sampleRate, bufferSize, paNoFlag, audioCallback, &mNodeManager);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            mNodeManager.setInputChannelCount(inputChannelCount);
            mNodeManager.setOutputChannelCount(outputChannelCount);
            mNodeManager.setSampleRate(sampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            if (error == paNoError)
            {
                PaDeviceInfo inputInfo = getDeviceInfo(inputDevice);
                PaDeviceInfo outputInfo = getDeviceInfo(outputDevice);
                Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", inputInfo.name, outputInfo.name, inputChannelCount, outputChannelCount, sampleRate, bufferSize);
            }
        }
        
        
        void AudioDeviceManager::startDefaultDevice(int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize)
        {
            auto error = Pa_OpenDefaultStream(&mStream, inputChannelCount, outputChannelCount, paFloat32 | paNonInterleaved, sampleRate, bufferSize, audioCallback, &mNodeManager);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            mNodeManager.setInputChannelCount(inputChannelCount);
            mNodeManager.setOutputChannelCount(outputChannelCount);
            mNodeManager.setSampleRate(sampleRate);
            
            error = Pa_StartStream(mStream);
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            
            if (error == paNoError)
                Logger::info("Portaudio default stream started: %i inputs, %i outputs, samplerate %i, buffersize %i", inputChannelCount, outputChannelCount, int(sampleRate), bufferSize);
        }
        
        
        
        void AudioDeviceManager::stop()
        {
            if (mStream && Pa_IsStreamActive(mStream) == 1)
            {
                Pa_StopStream(mStream);
                Pa_CloseStream(mStream);
				mStream = nullptr;
            }
            Logger::info("Portaudio stopped");
        }
        
        
        bool AudioDeviceManager::isActive()
        {
            return (Pa_IsStreamActive(mStream) == 1);
        }
        
        
    }
}
