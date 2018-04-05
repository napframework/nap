#pragma once

// third party includes
#include <portaudio.h>

// Nap includes
#include <nap/service.h>
#include <utility/safeptr.h>
#include <utility/threading.h>

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
            
            
            /**
             * This function is typically called by a hardware callback from the device to perform all the audio processing.
             * It performs memory management and processes a lockfree event queue before it invokes the @NodeManager::process() to process audio.
             * @param inputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param outputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param framesPerBuffer: the number of samples that has to be processed per channel
             */
            void audioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);
            
            /**
             * Enqueue a task to be executed within the process() method for thread safety
             */
            void execute(TaskQueue::Task task) { mAudioCallbackTaskQueue.enqueue(task); }
            
            /**
             * Constructs an object managed by a @SafeOwner that will dispose the object in the AudioService's @TrashBin when it is no longer used.
             */
            template <typename T, typename... Args>
            utility::SafeOwner<T> makeSafe(Args&&... args)
            {
                auto owner = utility::SafeOwner<T>(mTrashBin, new T(std::forward<Args>(args)...));
                return owner;
            }
            
        private:
            NodeManager mNodeManager; ///< The node manager that performs the audio processing.
            
            AudioDevice mInterface; ///< The audio interface representing the hardware or plugin interface
            
            nap::TaskQueue mAudioCallbackTaskQueue; // Queue with lambda functions to be executed on the next audio callback
            
            // TrashBin with nodes that are no longer used and that can be cleared and destructed safely on the next audio callback.
            // Clearing is performed by the NodeManager on the audio callback to make sure the node can not be destructed while it is being processed.
            utility::TrashBin mTrashBin;
        };
        
        
    }
}
