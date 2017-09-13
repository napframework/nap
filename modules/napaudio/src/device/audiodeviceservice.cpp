// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audioservice.h"
#include "audiointerface.h"

namespace nap {
    
    namespace audio {
        
        
        AudioService::~AudioService()
        {
            auto error = Pa_Terminate();
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            Logger::info("Portaudio terminated");
        }
        
        
        bool AudioService::init(nap::utility::ErrorState& errorState)
        {
            auto error = Pa_Initialize();
            if (error != paNoError)
            {
                std::string message = "Portaudio error: " + std::string(Pa_GetErrorText(error));
                errorState.fail(message);
                return false;
            }
            
            Logger::info("Portaudio initialized.");
            printDevices();
            
            return true;
        }
        
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<AudioInterfaceCreator>(*this));
        }
        
        
        unsigned int AudioService::getDeviceCount()
        {
            return Pa_GetDeviceCount();
        }
        
        
        const PaDeviceInfo& AudioService::getDeviceInfo(unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(deviceIndex);
        }
        
        
        std::vector<const PaDeviceInfo*> AudioService::getDevices()
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                result.emplace_back(&getDeviceInfo(i));
            }
            return result;
        }
        
        
        void AudioService::printDevices()
        {
            Logger::info("Available audio devices on this system:");
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                const PaDeviceInfo& info = *Pa_GetDeviceInfo(i);
                nap::Logger::info("%i: %s %i inputs %i outputs", i, info.name, info.maxInputChannels, info.maxOutputChannels);
            }
        }
        
        
        std::string AudioService::getDeviceName(unsigned int deviceIndex)
        {
            assert(deviceIndex < getDeviceCount());
            return getDeviceInfo(deviceIndex).name;
        }
        
    }
}

RTTI_DEFINE(nap::audio::AudioService)
