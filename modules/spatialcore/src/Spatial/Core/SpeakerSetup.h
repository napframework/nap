#pragma once

// Audio includes
#include <audio/utility/safeptr.h>

// Nap includes
#include <nap/resource.h>
#include <nap/signalslot.h>


namespace nap
{

    // Forward declarations
    namespace audio
    {
        class Process;
        class AudioService;
    }


    namespace spatial
    {

        // Forward declarations
        class SpatializationComponentInstance;
        class SpatialService;
        class Particle;

        /**
         * An arrangement of speakers that all sound objects can be projected onto.
         * This can be any multispeaker system, a stereo system or a binaural system.
         */
        class NAPAPI SpeakerSetup : public Resource {
            RTTI_ENABLE(Resource)

            friend class SpatializationComponentInstance;
            friend class SpatialService;
        public:
            /**
             * Constructor takes the @SpatialService that this resource will register itself with.
             */
            SpeakerSetup(SpatialService& service);
            virtual ~SpeakerSetup();
            

            // Inherited from Resource
            bool init(utility::ErrorState& errorState) override;

            /**
             * This method has to be overridden by descendants.
             * It needs to register the particle with this object and construct a system to make the particle sound through the speaker system.
             * In order to do so it needs to connect to the following signals that the @Particle exposes:
             * - activeChangedSignal
             * - getSpatialOutput().transformChangedSignal
             * - getSpatialOutput().outputPinChangedSignal
             */
            virtual void particleAdded(SpatializationComponentInstance&, Particle&) = 0;

            /**
             * This method has to be overridden by descendants.
             * It needs to remove the particle from this object's registry.
             */
            virtual void particleRemoved(SpatializationComponentInstance&, Particle&) = 0;

			/**
			 * Called during the SpatialService::update.
			 */
            virtual void update() { }

            SpatialService& getSpatialService() { return mService; }
            audio::AudioService& getAudioService();

            void setActive(bool active);

			/**
			 * @return Returns whether this speaker setup is active or not. When false, the SpatialService removes this component from the list of active speaker setups, and removes all of the known particles to reduce CPU cost.
			 */
			bool getIsActive() const { return mIsActive; }

			/**
			 * @return Returns the display name for use within the monitor GUI.
			 */
			virtual const char * getDisplayName() const = 0;

            /**
             * Called when the speakersetup is disconnected by the UI.
             */
            virtual void onDisconnect() { }

            /**
             * Returns the process that processes the DSP for all the speakers
             */
            template <typename T> T* getProcess() {
                if (mProcess == nullptr)
                    mProcess = createProcess();
                return rtti_cast<T>(mProcess.getRaw());
            }
            
            /**
             * Has to be overwritten to create the parent process for this speaker setup's DSP processing.
             */
            virtual audio::SafeOwner<audio::Process> createProcess();
            
        private:
            // Internally used by SpatialService to connect all SpatializationComponent instances
            void connect(SpatializationComponentInstance& component);
            // Internally used by SpatialService to disconnect all SpatializationComponent instances
            void disconnect(SpatializationComponentInstance& component);

        private:
            nap::Slot<SpatializationComponentInstance&, Particle&> particleCreatedSlot = {[&](SpatializationComponentInstance& component, Particle& particle){ particleCreated(component, particle); } };
            void particleCreated(SpatializationComponentInstance& component, Particle& particle);

            SpatialService& mService;
            
            audio::SafeOwner<audio::Process> mProcess = nullptr;

            bool mIsActive = false;
        };

    }

}
