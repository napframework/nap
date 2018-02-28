#pragma once

// third party includes
#include <portaudio.h>

// Nap includes
#include <nap/service.h>

// Audio includes
#include "audiodevice.h"

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Service that provides audio input and output processing directly for hardware audio devices.
         * Provides static methods to poll the current system for available audio devices using portaudio.
         */
        class NAPAPI AudioService final : public Service
        {
            RTTI_ENABLE(nap::Service)
            
        public:
            AudioService();
            
            ~AudioService();
            
            /**
             * Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            NodeManager& getNodeManager();
            
            /**
             * Initializes portaudio.
             */
            bool init(nap::utility::ErrorState& errorState) override;
            
			/**
			 *	Shutdown portaudio
			 */
			void shutdown() override;

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
             * @return: name of the specified host API
             */
            std::string getHostApiName(unsigned int hostApiIndex);
            
            /**
             * @param hostApiIndex: the number of the host api
             * @return: the number of all available audio devices for a certain host api, the total number contains both input and output devices separately.
             */
            unsigned int getDeviceCount(unsigned int hostApiIndex);
            
            /**
             * Returns information of an audio device in a PaDeviceInfo struct defined by portaudio.
             * @param hostApiIndex: the number of the host api
             * @param deviceIndex: the number of the device
             */
            const PaDeviceInfo& getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex);
            
            /** 
             * Returns information on all the available devices
             * @param hostApiIndex: the number of the host api
             */
            std::vector<const PaDeviceInfo*> getDevices(unsigned int hostApiIndex);
            
            /** 
             * Prints the number and name of all available audio devices to the console
             */
            void printDevices();
            
            /** 
             * @return the name of an available device specified by host api and device number
             * @param hostApiIndex: the number of the host api
             * @param deviceIndex: the number of the devie
             */
            std::string getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex);
            
            /**
             * Returns the device index for a device for a certain host API specified both by name.
             * Uses case insensitive search.
             * Returns -1 if the device specified was not found.
             */
            int getDeviceIndex(const std::string& hostApi, const std::string& device);
            
            
            AudioDevice mInterface;
        };
        
        
    }
}
