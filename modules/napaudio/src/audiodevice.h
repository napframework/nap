#pragma once

// third party includes
#include <portaudio.h>

namespace nap {
    
    namespace audio {
        
        /**
         * This class provided static methods to poll the current system for available audio devices using portaudio.
         */
        class NAPAPI AudioDeviceManager {
        public:
        public:
            /**
             * @return: the number of all available audio devices, the total number contains both input and output devices separately.
             */
            static unsigned int getDeviceCount();
            
            /**
             * Returns information of an audio device in a PaDeviceInfo struct defined by portaudio.
             * @param deviceIndex: the number of the device
             */
            static const PaDeviceInfo& getDeviceInfo(unsigned int deviceIndex);
            
            /** 
             * Returns information on all the available devices
             */
            static std::vector<const PaDeviceInfo*> getDevices();
            
            /** 
             * Prints the number and name of all available audio devices to the console
             */
            static void printDevices();
            
            /** 
             * @return the name of an available device specified by number
             * @param deviceIndex: the number of the devie
             */
            static const std::string& getDeviceName(unsigned int deviceIndex);
        };
        
        
    }
}
