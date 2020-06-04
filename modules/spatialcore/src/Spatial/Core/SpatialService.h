#pragma once


// Spatial includes
#include <Spatial/Core/RootProcess.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/node/inputnode.h>
#include <audio/node/levelmeternode.h>

// Nap includes
#include <nap/service.h>
#include <nap/signalslot.h>

// Std includes
#include <set>


namespace nap
{
    
    // Forward declarations
    namespace audio
    {
        class AudioService;
        class OutputPin;
        class NodeManager;
    }
    
    namespace spatial
    {
        
        // Forward declarations
        class SpeakerSetup;
        class MaskingComponentInstance;
        class MaskingOccluderComponentInstance;
        class SpatializationComponentInstance;
        class RootProcess;

        class NAPAPI SpatialService : public Service
        {
            RTTI_ENABLE(Service)
            
        public:
            // Default Constructor
            SpatialService(ServiceConfiguration* configuration);
            
            /**
             * Use this call to register service dependencies
             * A service that depends on another service is initialized after all it's associated dependencies
             * This will ensure correct order of initialization, update calls and shutdown of all services
             * @param dependencies rtti information of the services this service depends on
             */
            virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;
            
            /**
             * Initializes the service
             * @param errorState contains the error message on failure
             * @return if the video service was initialized correctly
             */
            virtual bool init(nap::utility::ErrorState& errorState) override;
            
            /**
             * Register object creators for resources (mostly panners)
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            /**
             * Invoked by core in the app loop. Update order depends on service dependency
             * This call is invoked after the resource manager has loaded any file changes but before
             * the app update call. If service B depends on A, A:s:update() is called before B::update()
             * @param deltaTime: the time in seconds between calls
            */
            virtual void update(double deltaTime) override;
            
            /**
             * Invoked when exiting the main loop, after app shutdown is called
             * Use this function to close service specific handles, drivers or devices
             * When service B depends on A, Service B is shutdown before A
             */
            virtual void shutdown() override;

            /**
             * @return the audio service used for audio playback.
             */
            audio::AudioService& getAudioService() { return *mAudioService; }

            /**
             * @return root process running on the audio callback.
             */
            RootProcess& getRootProcess() { return *mRootProcess; }

            /**
             * @return all presently registered speaker setups.
             */
             const std::set<SpeakerSetup*>& getSpeakerSetups() const { return mSpeakerSetups; };

             Signal<SpeakerSetup*> mSpeakerSetupRegisteredSignal; ///< Signal triggered when a new speaker setup is registered.
             Signal<SpeakerSetup*> mSpeakerSetupUnregisteredSignal; ///< Signal triggered when a speaker setup is destroyed and unregistered.

             /**
              * @return all spatialization components currently present in the system.
              */
             const std::set<SpatializationComponentInstance*>& getSpatializationComponents() const { return mSpatializationComponents; }

			 Signal<SpatializationComponentInstance*> mSpatializationComponentRegisteredSignal; ///< Signal triggerted when a spatialization component is registered
			 Signal<SpatializationComponentInstance*> mSpatializationComponentUnregisteredSignal; ///< Signal triggered when a spatialization component is unregistered

             /**
              * @return all masking components currently present in the system
              */
             const std::set<MaskingComponentInstance*>& getMaskingComponents() const { return mMaskingComponents; }

            /**
             * @return all masking occluder components currently present in the system
             */
            const std::set<MaskingOccluderComponentInstance*>& getMaskingOccluderComponents() const { return mMaskingOccluderComponents; }

			/**
			 * @return audio output pin containing the audio signal comming in through the ADC
			 */
			audio::OutputPin& getInput(int channel);

			/**
			 * @return current input level for specified ADC input channel
			 */
            float getInputLevel(int channel);

			/**
			 * @return number of ADC input channels
			 */
			int getInputChannelCount() const;

			/**
			 * Signal triggered when the number of ADC input channels changes.
			 */
			Signal<int> mInputChannelCountChangedSignal;
            
            /**
             * Signal triggered when the number of ADC output channels changes.
             */
            Signal<int> mOutputChannelCountChangedSignal;
            
            
            /**
             * Used by @SpeakerSetup to register itself with the service.
             */
            void registerSpeakerSetup(SpeakerSetup& panner);
            
            /**
             * Used by @SpeakerSetup to unregister itself with the service.
             */
            void unregisterSpeakerSetup(SpeakerSetup& panner);
            
            /**
             * Used by @SpatializationComponentInstance to register itself with the service and to connect to all @SpeakerSetup objects.
             */
            void connectSpatializationComponent(SpatializationComponentInstance& component);
            
            /**
             * Used by @SpatializationComponentInstance to unregister itself with the service and to disconnect from all @SpeakerSetup objects.
             */
            void disconnectSpatializationComponent(SpatializationComponentInstance& component);

            /**
             * Used by @MaskingComponentInstance to register itself with the service.
             */
            void registerMaskingComponent(MaskingComponentInstance& component);
			
            /**
             * Used by @MaskingComponentInstance to unregister itself with the service.
             */
            void unregisterMaskingComponent(MaskingComponentInstance& component);

            /**
             * Used by @MaskingOccluderComponentInstance to register itself with the service.
             */
            void registerMaskingOccluderComponent(MaskingOccluderComponentInstance& component);
			
            /**
             * Used by @MaskingOccluderComponentInstance to unregister itself with the service.
             */
            void unregisterMaskingOccluderComponent(MaskingOccluderComponentInstance& component);
            
        private:
            /**
             *  Initializes the VU level meters for each audio device input channel.
             */
            void initLevelMeters();
            
            // When the number of audio input channels changes the input VU meters have to be reinitialized
            Slot<audio::NodeManager&> mChannelCountChangedSlot = { this, &SpatialService::channelCountChanged };
            void channelCountChanged(audio::NodeManager&);

            std::set<SpeakerSetup*> mSpeakerSetups; // List with all registered panners
            std::set<SpatializationComponentInstance*> mSpatializationComponents; // List with all registered spatialization components
            std::set<MaskingComponentInstance*> mMaskingComponents; // List with all registered masking components
            std::set<MaskingOccluderComponentInstance*> mMaskingOccluderComponents; // List with all registered masking occluder components
            audio::AudioService* mAudioService = nullptr;            
            std::vector<audio::SafeOwner<audio::InputNode>> mInputNodes;
            std::vector<audio::SafeOwner<audio::LevelMeterNode>> mInputLevelMeters;

            audio::SafeOwner<RootProcess> mRootProcess = nullptr;
        };
        
    }
}
