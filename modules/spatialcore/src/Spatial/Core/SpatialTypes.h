#pragma once


// Nap includes
#include <nap/signalslot.h>

// Rtti includes
#include <rtti/rtti.h>

// Glm includes
#include <glm/glm.hpp>


namespace nap
{

    // Forward declarations
    namespace audio
    {
        class OutputPin;
    }

    namespace spatial
    {


        /**
         * Defines a rectangular three dimensional area in space.
         */
        class NAPAPI SpatialTransform {
            RTTI_ENABLE()
        public:
            SpatialTransform() = default;
            
            SpatialTransform(glm::vec3 position, glm::vec3 scale, glm::vec4 rotation) : mPosition(position), mScale(scale), mRotation(rotation) {}
        	virtual ~SpatialTransform() { }
			
            glm::vec3 mPosition = { 0, 0, 0 }; ///< The center position
            glm::vec3 mScale = { 0, 0, 0 }; ///< The dimensions of the rectangular space
            glm::vec4 mRotation = { 0, 1, 0, 0 }; ///< The rotation
        };


        /**
         * Represents a mono audio signal positioned in a region in space.
         */
        class NAPAPI SpatialOutput {
            RTTI_ENABLE()
        public:
            /**
             * Constructor takes an audio output pin, and the transform properties defining a rectangular region in space.
             */
            SpatialOutput(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale);
            virtual ~SpatialOutput() { }

            /**
             * Set the region in space that the audio signal has to be projected onto.
             * Each time this is called the @transformChangedSignal will be emitted.
             */
            void setTransform(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale);

            /**
             * Return the transform of the output.
             */
            const SpatialTransform& getTransform() const { return mTransform; }
            
            /**
             * Return the center position of the output in space.
             */
            const glm::vec3& getPosition() const { return mTransform.mPosition; }

            /**
             * Return the rotation of the output.
             */
            const glm::vec4& getRotation() const { return mTransform.mRotation; }

            /**
             * Return the scale (dimensions) of the output.
             */
            const glm::vec3& getScale() const { return mTransform.mScale; }

            /**
             * Set the output pin containing the audio signal that will be associated with this output.
             * Each time this is called the @outputPinChangedSignal will be emitted.
             */
            void setOutputPin(audio::OutputPin& pin);

            /**
             * Return the audio signal that's associated with this output.
             */
            audio::OutputPin* getOutputPin() const { return mOutputPin; }

            /**
             * A signal emitted each time this output's transform property changes.
             */
            Signal<const SpatialOutput&>* getTransformChangedSignal() { return &transformChangedSignal; }

            /**
             * A signal emitted each time the audio output pin has been replaced by a new one.
             */
            Signal<const SpatialOutput&>* getOutputPinChangedSignal() { return &outputPinChangedSignal; }

        private:
            SpatialTransform mTransform;
            audio::OutputPin* mOutputPin = nullptr;
            Signal<const SpatialOutput&> transformChangedSignal;
            Signal<const SpatialOutput&> outputPinChangedSignal;
        };


        /**
         * Represents the smallest unity of sound within a sound object.
         * The particle internally consists of a SpatialOutput that points to a mono audio signal and defines its region in space.
         * Because particles are normally preallocated during initialization of a sound object the particle can be activated or deactivated.
         * The @SpeakerSetup resources take care of making all active particles within sound objects to be played through the speakers.
         */
        class NAPAPI Particle {
            RTTI_ENABLE()
        public:
            /**
             * Constructor takes the audio output pin that contains this particle's audio signal, transform properties defining the particle's region in space and wether the particle is active after creation.
             */
            Particle(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale, bool active = false);
            virtual ~Particle() { }

            /**
             * Set wether the particle is currently active. Only active particle's are processed and audible through SpeakerSetups.
             */
            void setActive(bool active);

            /**
             * Returns wether the particle is currently active.
             */
            bool isActive() const { return mIsActive; }

            /**
             * Set the transform properties that define the region in space that the particle encompasses.
             */
            void setTransform(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale);

            /**
             * Return the transform of the particle.
             */
            const SpatialTransform& getTransform() const { return mOutput.getTransform(); }
            
            /**
             * Return the center position of the particle in space.
             */
            const glm::vec3& getPosition() const { return mOutput.getPosition(); }

            /**
             * Return the rotation of the particle.
             */
            const glm::vec4& getRotation() const { return mOutput.getRotation(); }

            /**
             * Return the scale (dimensions) of the particle.
             */
            const glm::vec3& getScale() const { return mOutput.getScale(); }

            /**
             * Set the particle's audio output signal.
             */
            void setOutputPin(audio::OutputPin& pin) { mOutput.setOutputPin(pin); }

            /**
             * Return the particle's audio output signal.
             */
            audio::OutputPin* getOutputPin() const { return mOutput.getOutputPin(); }

            /**
             * A signal emitted each time this output's transform property changes.
             */
            Signal<const SpatialOutput&>* getTransformChangedSignal() { return mOutput.getTransformChangedSignal(); }

            /**
             * A signal emitted each time the audio output pin has been replaced by a new one.
             */
            Signal<const SpatialOutput&>* getOutputPinChangedSignal() { return mOutput.getOutputPinChangedSignal(); }

            /**
             * Signal emitted everytime the particle has been activated or deactivated.
             */
            Signal<Particle&>* getActiveChangedSignal() { return &activeChangedSignal; }

        private:
            SpatialOutput mOutput;
            bool mIsActive = false;
            Signal<Particle&> activeChangedSignal;
        };

    }

}
