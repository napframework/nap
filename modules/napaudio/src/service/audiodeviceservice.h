#pragma once

// third party includes
#include <portaudio.h>

// Nap includes
#include <nap/service.h>

// Audio includes
#include "audioservice.h"
#include "audiodevice.h"

namespace nap {
    
    namespace audio {
        
        /**
         * This class provided static methods to poll the current system for available audio devices using portaudio.
         */
        class NAPAPI AudioDeviceService : public AudioService
        {
            RTTI_ENABLE(nap::audio::AudioService)
            
        public:
            AudioDeviceService();
            
            ~AudioDeviceService();
            
            NodeManager& getNodeManager() override final;
            
            /**
             * Initializes portaudio.
             */
            bool init(nap::utility::ErrorState& errorState);
            
            /**
             * @return: the number of available host APIs ont this system
             */
            unsigned int getHostApiCount();
            
            /**
             * Returns information about a given host api
             * @param hostApiIndex:
             * @return: struct containing information about the specified host api
             */
            const PaHostApiInfo& getHostApiInfo(unsigned int hostApiIndex);
            
            /**
             * Returns information on all available host apis
             */
            std::vector<const PaHostApiInfo*> getHostApis();
            
            /**
             * @return: the number of all available audio devices, the total number contains both input and output devices separately.
             */
            unsigned int getDeviceCount();
            
            /**
             * Returns information of an audio device in a PaDeviceInfo struct defined by portaudio.
             * @param deviceIndex: the number of the device
             */
            const PaDeviceInfo& getDeviceInfo(unsigned int deviceIndex);
            
            /** 
             * Returns information on all the available devices
             */
            std::vector<const PaDeviceInfo*> getDevices();
            
            /** 
             * Prints the number and name of all available audio devices to the console
             */
            void printDevices();
            
            /** 
             * @return the name of an available device specified by number
             * @param deviceIndex: the number of the devie
             */
            std::string getDeviceName(unsigned int deviceIndex);
            
            
            AudioDevice mInterface;
        };
        
        
    }
}
