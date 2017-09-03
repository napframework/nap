#pragma once

// std library includes
#include <vector>

// third party includes
#include <portaudio.h>

// nap includes
#include <utility/dllexport.h>

// audio includes
#include "audionodemanager.h"


namespace nap {
    
    namespace audio {
        
        
        // Forward declarations
        class AudioDeviceManager;
        
        
        class NAPAPI AudioInterface : public rtti::RTTIObject {
            RTTI_ENABLE(rtti::RTTIObject)
            
        public:
            AudioInterface();
            ~AudioInterface();
            
            bool init(utility::ErrorState& errorState) override;
            
            void start();
            void stop();
            bool isActive();
            
            AudioNodeManager& getNodeManager() { return mNodeManager; }
            
        public:
            bool mUseDefaultDevice = true;
            int mInputDevice = 0;
            int mOutputDevice = 0;
            int mInputChannelCount = 1;
            int mOutputChannelCount = 2;
            float mSampleRate = 44100;
            int mBufferSize = 256;
            
        private:
            void startDefaultDevice();
            
            AudioNodeManager mNodeManager;
            
            bool mInitialized = false;
            PaStream* mStream = nullptr;
        };
        
        
        class NAPAPI AudioDeviceManager {
        public:
        public:
            static bool init();
            static void terminate();
            
            static unsigned int getDeviceCount();
            static const PaDeviceInfo& getDeviceInfo(unsigned int deviceIndex);
            static std::vector<const PaDeviceInfo*> getDevices();
            static void printDevices();
            static const std::string& getDeviceName(unsigned int deviceIndex);
        };
        
        
    }
}
