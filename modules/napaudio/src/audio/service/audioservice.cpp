// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "audioservice.h"
#include "audiodevice.h"

#include <audio/core/graph.h>
#include <audio/core/voice.h>

// Third party includes
#include <mpg123.h>


RTTI_DEFINE_CLASS(nap::audio::AudioService)

namespace nap
{
    
    namespace audio
    {
        
        AudioService::AudioService() : mInterface(*this)
        {
            // Initialize mpg123 library
            mpg123_init();
        }

        
        AudioService::~AudioService()
        {
            // Uninitialize mpg123 library
            mpg123_exit();
        }
        
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(getNodeManager()));
            factory.addObjectCreator(std::make_unique<VoiceObjectCreator>(getNodeManager()));
        }

        
        NodeManager& AudioService::getNodeManager()
        {
            return mInterface.getNodeManager();
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
            
            // Initialize the audio device
            return mInterface.init(errorState);
        }
        
        
        unsigned int AudioService::getHostApiCount()
        {
            return Pa_GetHostApiCount();
        }
        
        
        const PaHostApiInfo& AudioService::getHostApiInfo(unsigned int hostApiIndex)
        {
            return *Pa_GetHostApiInfo(hostApiIndex);
        }
        
        
        std::vector<const PaHostApiInfo*> AudioService::getHostApis()
        {
            std::vector<const PaHostApiInfo*> result;
            for (auto i = 0; i < Pa_GetHostApiCount(); ++i)
            {
                result.emplace_back(&getHostApiInfo(i));
            }
            return result;
        }
        
        
        std::string AudioService::getHostApiName(unsigned int hostApiIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            return getHostApiInfo(hostApiIndex).name;
        }
        
        
        unsigned int AudioService::getDeviceCount(unsigned int hostApiIndex)
        {
            return getHostApiInfo(hostApiIndex).deviceCount;
        }
        
        
        const PaDeviceInfo& AudioService::getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex));
        }
        
        
        std::vector<const PaDeviceInfo*> AudioService::getDevices(unsigned int hostApiIndex)
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < getHostApiInfo(hostApiIndex).deviceCount; ++i)
            {
                result.emplace_back(&getDeviceInfo(hostApiIndex, i));
            }
            return result;
        }
        
        
        void AudioService::printDevices()
        {
            Logger::info("Available audio devices on this system:");
            for (auto hostApi = 0; hostApi < Pa_GetHostApiCount(); ++hostApi)
            {
                const PaHostApiInfo& hostApiInfo = *Pa_GetHostApiInfo(hostApi);
                nap::Logger::info("%s:", hostApiInfo.name);
                for (auto device = 0; device < hostApiInfo.deviceCount; ++device)
                {
                    auto index = Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device);
                    const PaDeviceInfo& info = *Pa_GetDeviceInfo(index);
                    nap::Logger::info("%i: %s %i inputs %i outputs", device, info.name, info.maxInputChannels, info.maxOutputChannels);
                }
            }
        }
        
        
        std::string AudioService::getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            assert(deviceIndex < getDeviceCount(hostApiIndex));
            return getDeviceInfo(hostApiIndex, deviceIndex).name;
        }
        
        
        int AudioService::getDeviceIndex(const std::string& hostApi, const std::string& device)
        {
            for (auto hostApiIndex = 0; hostApiIndex < getHostApiCount(); ++hostApiIndex)
            {
                auto hostApiInfo = getHostApiInfo(hostApiIndex);
                if (nap::utility::toLower(hostApi) == nap::utility::toLower(hostApiInfo.name))
                {
                    for (auto deviceIndex = 0; deviceIndex < hostApiInfo.deviceCount; ++deviceIndex)
                    {
                        auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                        if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name))
                            return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
                    }
                }
            }
            return -1;
        }


		void AudioService::shutdown()
		{
			auto error = Pa_Terminate();
			if (error != paNoError)
				Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
			Logger::info("Portaudio terminated");
		}
    }
}

RTTI_DEFINE_CLASS(nap::audio::AudioService)
