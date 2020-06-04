#pragma once

// Audio includes
#include <audio/utility/audiotypes.h>

// Nap includes
#include <nap/signalslot.h>
#include <component.h>

// Glm includes
#include <glm/glm.hpp>

// Std includes
#include <set>


namespace nap
{

	class TransformComponentInstance;
    namespace audio
    {
        class OutputPin;
    }

    namespace spatial
    {

        // Forward declarations
        class SpatializationComponentInstance;
        class Particle;
        class SpatialService;



        /**
         * Component that provides particle information to the @SpatialService and all registered @SpeakerSetup objects. Manages all @Particle objects within a spatial sound entity.
         */
        class NAPAPI SpatializationComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(SpatializationComponent, SpatializationComponentInstance)

        public:
            SpatializationComponent() : Component() { }
            
        private:
        };


        /**
         * Instance of a component that provides particle information to the @SpatialService and all registered @SpeakerSetup objects. Manages all @Particle objects within a spatial sound entity.
         */
        class NAPAPI SpatializationComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
			
            friend class SpatializationComponent;
        public:
            SpatializationComponentInstance(EntityInstance& entity, Component& resource);
            ~SpatializationComponentInstance();

            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            /**
             * Add a @Particle to the sound object.
             * Results in the mParticleCreatedSignal to be emitted.
             * @param pin: an output pin from a DSP network that contains the audio signal of the new particle
             * @param position: center position of the particle
             * @param rotation: the rotation of the particle region
             * @param scale: the dimensions of the particle region
             * @param active: wether the particle is active
             */
            Particle* addParticle(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale, bool active);

            /**
             * Returns the number of particles.
             */
            int getParticleCount() const { return mParticles.size(); }
			
            /**
             * Returns the number of active particles.
             */
            int calculateActiveParticleCount() const;

            /**
             * Returns a particle with a give index. Returns nullptr when the index is out of bounds.
             */
            Particle* getParticle(unsigned int index) const;

            /**
             * Accessor for all particles.
             */
            const std::vector<std::unique_ptr<Particle>>& getParticles() { return mParticles; }

            /**
             * Returns all spatial sound components connected as inputs.
             */
            std::vector<SpatializationComponentInstance*> getInputs() const;

            /**
             * Returns all outputs that this sound component is connected to as input.
             */
            const std::set<SpatializationComponentInstance*> getOutputs() const { return mOutputs; }

            /**
             * Returns the input gain of the given SpatialSoundComponentInstance connected as input.
             * Returns 0 if the input has not been found.
             */
            audio::ControllerValue getInputGain(SpatializationComponentInstance& input) const;

            /**
             * Set the input gain of a SpatialSoundComponentInstance connected as input.
             */
            void setInputGain(SpatializationComponentInstance& input, audio::ControllerValue gain);

            /**
             * Connects &input as an input for this spatial sound component.
             * Results in the mInputConnectedSignal to be emitted.
             */
            void connectInput(SpatializationComponentInstance* input, audio::ControllerValue gain = 1.0f);

            /**
             * Return the gain value that will be used to play the sound object through the speaker setup.
             */
            audio::ControllerValue getDryGain() const { return mDryGain; }

            /**
             * Set the gain value that will be used to play the sound object through the speaker setup.
             */
            void setDryGain(audio::ControllerValue gain);

            // --- Signals --- //

            /**
             * Returns a signal that will be emitted when a new sound object has been connected to this component as output.
             * The signal will pass the SpatialSoundComponent of the newly connected sound object as parameter.
             */
            Signal<SpatializationComponentInstance&>* getInputConnectedSignal() { return &mInputConnectedSignal; }

            /**
             * Returns a signal that will be emitted when a new sound space has been connected to this component as output.
             * The signal will pass the SpatialSoundComponent of the newly connected sound space as parameter.
             */
            Signal<SpatializationComponentInstance&>* getOutputConnectedSignal() { return &mOutputConnectedSignal; }
            
            /**
             * Returns a signal that will be emitted when a new particle has been created within this sound object.
             */
            Signal<SpatializationComponentInstance&, Particle&>* getParticleCreatedSignal() { return &mParticleCreatedSignal; }

            /**
             * Returns a signal that will be emitted when the input gain of @input has been changes.
             */
            Signal<audio::ControllerValue>* getInputGainChangedSignal(SpatializationComponentInstance* input);

            /**
             * Returns the signal that will be emitted when the gain value changes that will be used to play the sound object through the speaker setup.
             */
            Signal<audio::ControllerValue>* getDryGainChangedSignal() { return &mDryGainChangedSignal; }
			
            
            SpatialService& getSpatialService() { return *mSpatialService; }

        private:
            // Helper class to keep track of one connected input.
            class Input
            {
            public:
                Input(SpatializationComponentInstance* input, audio::ControllerValue gain) : mInput(input), mGain(gain) { }
                SpatializationComponentInstance* mInput = nullptr;
                audio::ControllerValue mGain = 1.0f;
                Signal<audio::ControllerValue> mInputGainChangedSignal;
            };

            std::vector<std::unique_ptr<Particle>> mParticles;
            std::vector<std::unique_ptr<Input>> mInputs;
            std::set<SpatializationComponentInstance*> mOutputs;
            audio::ControllerValue mDryGain = 1.0f;
			
            Signal<SpatializationComponentInstance&, Particle&> mParticleCreatedSignal; // Signal emitted when a new particle has been created.
            Signal<SpatializationComponentInstance&> mInputConnectedSignal;
            Signal<SpatializationComponentInstance&> mOutputConnectedSignal;
            Signal<audio::ControllerValue> mDryGainChangedSignal;

            SpatialService* mSpatialService = nullptr;
            TransformComponentInstance * mTransformComponent = nullptr;
        };



    }

}
