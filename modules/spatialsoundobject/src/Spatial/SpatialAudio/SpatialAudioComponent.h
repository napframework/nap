#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/Effect.h> // inside resourceptr
#include <Spatial/SpatialAudio/SpatialSource.h>
#include <Spatial/Core/SpatialTypes.h> // decided to never forward declare SpatialTypes
#include <Spatial/Utility/ParameterTypes.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainControl.h>
#include <Spatial/Audio/FastMixNode.h>

// Audio includes
#include <audio/core/multichannel.h>

// Nap includes
#include <component.h>

namespace nap
{

    // Forward declarations
    class ParameterComponentInstance;
    class Parameter;

    namespace spatial
    {

        // Forward declarations
        class SpatialTransformationComponentInstance;
        class MeasurementComponentInstance;
        class SpatializationComponentInstance;
        class SpatialSource;
        class SpatialSourceInstance;
        class DistributedSourceInstance;
        class RootProcess;
        class SpatialAudioComponentInstance;
        class SpatialService;


        /**
         * This component provides an implementation of the core spatial audio framework and adds DSP functionality for particle processing on top of @SpatializationComponent.
         * The DSP chain encapsulated by SpatialAudioComponent manages one or more @SpatialSource objects that provide audio input and an @EffectChain containing three serial chains of @SpatialEffect objects that process the audio signal:
         * - The InputEffects: a chain of effects that is applied separately to the output of each SpatialSource.
         * - The main Effects chain: a chain of effects applied to a mix of the different outputs for each source coming from InputEffects. The output of this chain is fed into the @MixdownComponent to provide a mono mixdown of the SpatialAudioComponent's output.
         * - the PerceptionEffects chain: a chain of effects that is applied to the output of the main effects chain before it is sent to the @SpeakerSetup to be projected to the speakers. The PerceptionEffects don't have any influence on the mixdown.
         *
         * Currently @SpatialSource objects can be added to the component using the @SpatialAudioComponentInstance::addSource() method, and effects in the chains can be specified as resources in the @SpatialAudioComponent resource.
         */
        class NAPAPI SpatialAudioComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(SpatialAudioComponent, SpatialAudioComponentInstance)

        public:
            SpatialAudioComponent() : Component() { }

            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

            // Note: due to a Napkin bug, these resources can't be of type EffectChain. If this bug has been resolved these resources can become 'EffectChains' instead of vectors and the mInputEffects and mEffects members of SpatialAudioComponentInstance can be removed.
            std::vector<ResourcePtr<EffectBase>> mInputEffects; ///< Property. The Effects 'per input per particle'.
            std::vector<ResourcePtr<EffectBase>> mEffects; ///< Property. The Effects 'per particle', pre-mixdown.
            std::vector<ResourcePtr<EffectBase>> mPerceptionEffects; ///< Property. The Effects 'per particle', post-mixdown.
            int mMaxParticleCount = 128; ///< Property: 'MaxParticleCount'

        };


        /**
         * Instance of @SpatialAudioComponent
         */
        class NAPAPI SpatialAudioComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)

        public:
            SpatialAudioComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

            // Inherited from ComponentInstance
            bool init(utility::ErrorState& errorState) override;
            virtual void update(double deltaTime) override;

            /**
             * Returns an effect with name 'name'. It will search by this name in both the inputeffectchain and the effectchain, or return nullptr if it has not been found.
             */
            EffectInstanceBase* getEffectByName(std::string name)
            {
                EffectInstanceBase* effect;
                effect = mInputEffectChainInstance->getEffectByName(name);
                if(effect != nullptr)
                    return effect;

                return mEffectChainInstance->getEffectByName(name);
            }

            /**
             * Instantiates a new spatial source from a resource and adds it as input for the soundobject.
             * An enable parameter is automatically generated for the new source.
             * @param resource resource of the new @SpatialSource
             * @param errorState errors during the creation and adding will be logged here
             * @return true on success
             */
            bool addSource(SpatialSource& resource, utility::ErrorState& errorState);

            /**
             * Python overload for @addSource(SpatialSource& resource, utility::ErrorState& errorState).
             * Logs a warning on failure.
             * @param resource resource of the new @SpatialSource
             * @return true on success
             */
            bool addSource(SpatialSource* resource);

            /**
             * Adds an instance of a @SpatialSource as input for this sound object
             * @param source @SpatialSource instance to ba added as new input
             * @param enableParameterName name of the enable parameter for this source
             */
            void addSource(std::unique_ptr<SpatialSourceInstance> source, const std::string& enableParameterName = "enable");

            /**
             * Adds a test oscillator input to the sound object (used for stresstesting).
             */
            void addTestSignal();

            /**
             * Adds the ExternalInputSource for this sound object and adds a processor chain to the input effect chain.
             */
            void addExternalInput();

            /**
             * @return list of all sources for the sound object
             */
            const std::vector<std::unique_ptr<SpatialSourceInstance>>& getSources() const { return mSources; };

            /**
             * Returns the input level for given channel of the DistributedSource with given @index
             * @param index index of the source in the list in order of creation.
             * @param channel channel number of the input channels
             * @return input level of the source on given channel.
             */
            float getInputLevelForSource(int index, int channel);

            /**
             * Check wether a source is enabled/active
             * @param index of the source in the list in order of creation
             * @return true when the source is active/enabled
             */
            bool getSourceEnable(int index) { return mSourceEnableParameters[index]->mValue; }

        private:
            class ParticleActivator;

        private:
            /**
             * Updates all effects.
             */
            void updateEffectChains(double deltaTime);

            /**
             * Gets the particle transforms from SpatialTransformationComponent and sets the transforms and activations of the @Particles accordingly.
             */
            void updateParticles(double deltaTime);

            /**
             * Initialises parameters.
             */
            void initParameters();

            /**
             * Calls addSoundObjectSource() when an input has been connected to @SpatializationComponent.
             */
            Slot<SpatializationComponentInstance&> mInputConnectedSlot = {this,&SpatialAudioComponentInstance::addSoundObjectSource };

            /**
             * Activates/deactivates a processing chain for a certain source by index.
             * @param index  Index of the source.
             * @param active Indicates wether the source will be active.
             */
            void setSourceActive(unsigned int index, bool active);

            /**
             * Adds a SoundObjectSource and adds a processor chain to the input effect chain.
             */
            void addSoundObjectSource(SpatializationComponentInstance& inputObject);

            /**
             * Returns required channel count of the processor chains.
             */
            int getParticleCount() { return mParticleCount; }


        private:
            int mParticleCount = 0; // Number of particles.
            int mActiveParticleCount = 0; // Number of particles that is currently active and producing audio

            /**
             * SpatialSources are the inputs to the effect chains (external inputs or sound object inputs).
             */
            std::vector<std::unique_ptr<SpatialSourceInstance>> mSources;
            std::vector<ParameterBool*> mSourceEnableParameters; // Parameters indicating wether each source is enabled

            // Note: Due to a Napkin bug, embedded resources can't be of type EffectChain. If this bug has been resolved the SpatialAudioComponent resources can become 'EffectChains' instead of vectors and these members can be removed.
            EffectChain mInputEffectChain;
            EffectChain mEffectChain;
            EffectChain mPerceptionEffectChain;

            std::unique_ptr<EffectChainInstance> mInputEffectChainInstance = nullptr; // effects 'per input per particle'
            std::unique_ptr<EffectChainInstance> mEffectChainInstance = nullptr; // effects 'per particle'
            std::unique_ptr<EffectChainInstance> mPerceptionEffectChainInstance = nullptr; // effects 'per particle' post-mixdown

            std::unique_ptr<audio::ParallelNodeObjectInstance<audio::FastMixNode>> mMixer = nullptr; // the mixer that adds up the processor chains of the "InputEffectChain".
            std::unique_ptr<audio::ParallelNodeObjectInstance<audio::FastGainControlNode>> mParticleFader = nullptr; // gain nodes controlled by the particle activators, that fade in/out the particle signals.
            std::vector<std::unique_ptr<ParticleActivator>> mParticleActivators;

            // components
            SpatializationComponentInstance* mSpatializationComponent;
            MeasurementComponentInstance* mMeasurementComponent;
            ParameterComponentInstance* mParameterComponent;
            SpatialTransformationComponentInstance* mSpatialTransformationComponent;

            // audio service
            audio::AudioService* mAudioService = nullptr;
            SpatialService* mSpatialService = nullptr;

            // ____ ____ BELOW SHOULD BE MOVED AWAY FROM SpatialAudioComponent ____ ____
            // Parameters for SpatializationComponent (should move to SpatializationComponent & visualisation components)
            ParameterFloat* mDryGain;
            // ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____
        };


        /**
         * This object regulates activation/deactivation of a particle by fading it in and out, (de)activating it under the hood for the panner and registering and deregistering it with the root process for parallel (multicore) processing.
         */
        class NAPAPI SpatialAudioComponentInstance::ParticleActivator
        {
        public:
            ParticleActivator(Particle& particle, audio::FastGainControlNode& fader, RootProcess& rootProcess);
            void setActive(bool active);
            bool getActive() const { return mActive; }

        private:
            Slot<audio::ControllerValue> mDestinationReachedSlot = { this, &ParticleActivator::destinationReached };
            void destinationReached(audio::ControllerValue value);

            Particle& mParticle;
            audio::FastGainControlNode& mFader;
            RootProcess& mRootProcess;
            bool mActive = false;
        };

    }
}
