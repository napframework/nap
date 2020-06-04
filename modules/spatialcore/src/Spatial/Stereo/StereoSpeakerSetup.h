#pragma once

// Spatial includes
#include <Spatial/Audio/FastMixNode.h>
#include <Spatial/Core/SpeakerSetup.h>
#include <Spatial/Core/SpatializationComponent.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/node/outputnode.h>
#include <audio/node/stereopannernode.h>
#include <audio/node/gainnode.h>

// NAP includes
#include <rtti/factory.h>

namespace nap
{

    namespace spatial
    {

        // Forward declarations
        class SpatialService;
        class SpatialOutput;
        class SpatializationComponentInstance;
    
        /**
         * Dummy speaker setup for a stereo system.
         * Maps Particle's positions on the x axis to equal power panning.
         */
        class NAPAPI StereoSpeakerSetup : public SpeakerSetup {
            RTTI_ENABLE(SpeakerSetup)
			
            friend class StereoSpeakerSetupGui;
			
        public:
            StereoSpeakerSetup(SpatialService& service) : SpeakerSetup(service) { }
            virtual ~StereoSpeakerSetup() override;
            
            // Inherited from StereoSpeakerSetup
            bool init(utility::ErrorState& errorState) override;
            void particleAdded(SpatializationComponentInstance&, Particle&) override;
            void particleRemoved(SpatializationComponentInstance&, Particle&) override;
			
			/**
			 * @return The master volume value.
			 */
			audio::ControllerValue getMasterVolume() const { return mMasterVolume; }
			
			/**
			 * Sets the master volume
			 */
			void setMasterVolume(const audio::ControllerValue value) {
				mMasterVolume = value;
				mMasterVolumeChangedSignal(mMasterVolume);
			}
			
			Signal<audio::ControllerValue> & getMasterVolumeChangedSignal() { return mMasterVolumeChangedSignal; }
			
            virtual const char * getDisplayName() const override { return "Stereo"; }
            
            std::vector<int> mOutputChannels = { 0, 1 }; ///< property: 'OutputChannels' The output numbers of the DAC to send the output to.
            float mRegion = 5; ///< property: 'Region' The width of the region on the x axis to map between far left and far right.                        

        private:
            // Helper class to do the actual panning and distribution across the for one particle.
            class ParticleProcessor;
            
            // Connects a new @ParticleProcessor to the SpeakerSetup's output chain.
            void connectParticleProcessor(ParticleProcessor& particleProcessor);
            // Disconnects a new @ParticleProcessor to the SpeakerSetup's output chain.
            void disconnectParticleProcessor(ParticleProcessor& particleProcessor);

        private:
            std::vector<std::unique_ptr<ParticleProcessor>> mParticleProcessors;
            std::vector<audio::SafeOwner<audio::FastMixNode>> mMixers;
            std::vector<audio::SafeOwner<audio::OutputNode>> mOutputs;
            
			Signal<audio::ControllerValue> mMasterVolumeChangedSignal;
			
            audio::ControllerValue mMasterVolume = 0.f;
        };
        
        
        using StereoSpeakerSetupObjectCreator = rtti::ObjectCreator<StereoSpeakerSetup, SpatialService>;

        
        /**
         * Helper class to perform the actual panning for one @Particle.
         * Connects the Particle's output pin to a @StereoPannerNode and responds to the Particle's signals transformChangedSignal, outputPinChangedSignal and activeChangedSignal.
         */
        class StereoSpeakerSetup::ParticleProcessor
        {
        public:
            ParticleProcessor(StereoSpeakerSetup& speakerSetup, SpatializationComponentInstance& soundObject, Particle& particle);
            Particle* getParticle() { return mParticle; }
            
            audio::OutputPin& getLeftOutput();
            audio::OutputPin& getRightOutput();
            
        private:
            void pan(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale);
            void updateGain();
            
            Slot<const SpatialOutput&> transformChangedSlot = { this, &ParticleProcessor::transformChanged };
            Slot<const SpatialOutput&> outputPinChangedSlot = { this, &ParticleProcessor::outputPinChanged };
            Slot<Particle&> activeChangedSlot = { this, &ParticleProcessor::activeChanged };
            Slot<audio::ControllerValue> gainChangedSlot = { this, &ParticleProcessor::gainChanged };
            Slot<audio::ControllerValue> masterVolumeChangedSlot = { this, &ParticleProcessor::masterVolumeChanged };
            void transformChanged(const SpatialOutput&);
            void outputPinChanged(const SpatialOutput&);
            void activeChanged(const Particle&);
            void gainChanged(audio::ControllerValue gain);
            void masterVolumeChanged(audio::ControllerValue gain);
            
            Particle* mParticle = nullptr;
            audio::SafeOwner<audio::GainNode> mGainNode = nullptr;
            audio::SafeOwner<audio::StereoPannerNode> mStereoPannerNode = nullptr;
            StereoSpeakerSetup* mSpeakerSetup = nullptr;
            audio::ControllerValue mGain = 1.0f;
        };
        
        
    }
    
}
