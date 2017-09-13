#pragma once

// third party includes
#include <portaudio.h>

// nap includes
#include <nap/service.h>

namespace nap {
    
    namespace audio {
        
        /**
         * This class provided static methods to poll the current system for available audio devices using portaudio.
         */
        class NAPAPI AudioService : public nap::Service
        {
            RTTI_ENABLE(nap::Service)
            
        public:
            AudioService() = default;
            
            ~AudioService();
            
            /**
             * Initializes portaudio.
             */
            bool init(nap::utility::ErrorState& errorState);
            
            /**
             *	Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
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
            
        private:
            
        };
        
        
    }
}
