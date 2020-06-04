#pragma once

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Utility/ParameterTypes.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainNode.h>
#include <Spatial/Audio/FastMixNode.h>
#include <Spatial/Audio/EnvelopeFollowerNode.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/node/buffernode.h>
#include <audio/node/levelmeternode.h>

// Nap includes
#include <component.h>


namespace nap
{
 
    namespace audio
    {
        class AudioService;
    }
    
    namespace spatial
    {
        // Forward declarations
        class Parameter;
        class SpatialService;
        
        
        class MixdownComponentInstance;
        
        
        /**
         * Component that performs the DSP for a mixdown of a specified amount of channels of a sound object.
         * Particles can be added to the mixdown instance by another component using addParticle().
         * The mixdown is made by distributing the sound object's particles evenly over the mixdown's channel and scaling the gain of the mixdown accordingly with the amount of mixed particles.
         * The mixdown also has an @EnvelopeFollowerNode on its first channel of which the value can be requested through the getMeasuredLevel() function (used for spatial dynamics).
         */
        class NAPAPI MixdownComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(MixdownComponent, MixdownComponentInstance)
            
        public:
            MixdownComponent() : Component() { }
            
            // Inherited from Component
            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
            int mChannelCount = 1; ///> property: 'ChannelCount' Number of channels of the mixdown. For example: 1 for mono, 2 for stereo mixdowns.
        };

        
        /**
         * Instance of a @MixdownComponent.
         */
        class NAPAPI MixdownComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
            
        public:
            MixdownComponentInstance(EntityInstance& entity, Component& resource);
            ~MixdownComponentInstance();
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            // Inherited from ComponentInstance
            void update(double deltaTime) override;
            
            /**
             * Returns the current level of the level meter.
             */
            float getMeasuredLevel() const;

            /**
             * Returns the current output of the envelope follower.
             */
            float getEnvelopeFollowerOutput() const;

            /**
             * @return spatial output of the mixdown for @channel
             */
            SpatialOutput* getOutput(int channel);

            /**
             * @return number of channels of this mixdown
             */
            int getChannelCount() const;

            /**
             * Add a particle to the mixdown whose output signal will be mixed in the mixdown and whose transform information will influence the mixdown's transform.
             * @param particle the particle to be added
             * @param particleMixdownInput this parameter can be used in order to choose a different output pin from within the particle's DSP to mix into the mixdown instead of the particle's standard output. If this parameter is not used the particle's output pin will be used as a default.
             */
            void addParticle(Particle& particle, audio::OutputPin* particleMixdownInput = nullptr);

            /**
             * Set the attack time in ms of the envelope follower that measures the mixdowns output level.
             */
            void setEnvelopeFollowerAttack(float attack);

            /**
             * Set the decay time in ms of the envelope follower that measures the mixdowns output level.
             */
            void setEnvelopeFollowerRelease(float release);

        private:
            // Helper struct to administer an input particle and the pointer to the output pin from the particle's DSP that will be used as the mixdown's input
            struct ParticleInput
            {
                ParticleInput(Particle* particle, audio::OutputPin* mixdownInput) : mParticle(particle), mMixdownInput(mixdownInput) { }
                Particle* mParticle = nullptr;
                audio::OutputPin* mMixdownInput = nullptr;
            };
            
        private:
            // Respond to input particle's transform changes
            Slot<const SpatialOutput&> mParticleTransformChangedSlot = { this, &MixdownComponentInstance::particleTransformChanged };
            void particleTransformChanged(const SpatialOutput&) { mDirty = true; }

            // Respond to input particle's activation changes
            Slot<Particle&> mParticleActiveChangedSlot = { this, &MixdownComponentInstance::particleActiveChanged };
            void particleActiveChanged(Particle&);

            // Adjust the gains of the mixdown's channels to compensate for the number of input particles
            void adjustGains();

            // Helper method to find the channel number of the mixdown where the given particle's audio signal is mixed into.
            int getChannelForParticle(const Particle&) const;

            // Returns the number of particles that are mixed into the mixdown
            int getParticleCount() const;

            // Recalculates the transforms of each mixdown channel from the transforms of the particles on that channel.
            void updateTransforms();

            // Mixer for each channel in the mixdown to mix all input particles
            std::vector<audio::SafeOwner<audio::FastMixNode>> mMixNodes;
            // Gain for each channel in the mixdown to compensate for the number of particles mixed in
            std::vector<audio::SafeOwner<audio::FastGainNode>> mGainNodes;
            // Buffernode for each channel in order to allow for feedback routing between sound objects using mixdowns
            std::vector<audio::SafeOwner<audio::BufferNode>> mBufferNodes;

            // Envelope follower for first channel (channel 0) of the mixdown for exposing (for OSC output for example)
            audio::SafeOwner<audio::EnvelopeFollowerNode> mEnvelopeFollowerNode = nullptr;

            // Level meter for VU metering
            audio::SafeOwner<audio::LevelMeterNode> mLevelMeterNode = nullptr;

            // Particles in the mixdown sorted per mixdown channel
            std::vector<std::set<std::unique_ptr<ParticleInput>>> mParticles;

            // The actual mixdown output: a SpatialOutput for each channel
            std::vector<std::unique_ptr<SpatialOutput>> mMixdown;
            bool mDirty = false; // Indicates wether transforms chould be recalculated on the next update loop.
            
            ParameterFloat* mEnvelopeFollowerAttack;
            ParameterFloat* mEnvelopeFollowerRelease;

            MixdownComponent* mResource = nullptr;
            audio::AudioService* mAudioService = nullptr;
            SpatialService* mSpatialService = nullptr;
        };
        
    }
        
}
