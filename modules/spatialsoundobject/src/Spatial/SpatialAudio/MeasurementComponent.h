#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialSource.h> // in slot
#include <Spatial/Utility/ParameterTypes.h>

// Nap includes
#include <nap/signalslot.h>
#include <component.h>

// Glm includes
#include <glm/glm.hpp>

// Std includes
#include <atomic>


namespace nap
{
    namespace spatial
    {

        // Forward declarations
        class Particle;
        class SpatialTransform;
        class SpatialOutput;
        class SpatializationComponentInstance;
        class MeasurementComponentInstance;


        /**
         * Class that measures data of a single particle (for now only distance).
         * Emits a signal when the data changed.
         */
        class NAPAPI ParticleMeasurer {

            friend class MeasurementComponentInstance;

        public:
            ParticleMeasurer(Particle& particle, MeasurementComponentInstance& measurementComponent);

            const SpatialTransform& getTransform();
            const glm::vec3& getPosition();
            const glm::vec3& getDimensions();
            const glm::vec4& getRotation();
            const float& getHeight() { return mHeight; }

            float getDistanceToVantagePoint(){ return mDistanceToVantagePoint; }

            float getDistanceToInput(int inputIndex)
            {
                if(inputIndex >= mDistancesToInputs.size())
                    return 0;

                return mDistancesToInputs[inputIndex];
            }

            glm::vec3 getVantagePoint();

            float getGroundLevel();

            /**
             * Returns pointer to a signal that emits after data has been recalculated.
             */
            Signal<const ParticleMeasurer&>* getDataChangedSignal(){ return &mDataChangedSignal; }

            void addInputTransform(const SpatialTransform* inputTransform);


        private:
            /**
             * Recalculates data dependant on inputs. Called by MeasurementComponent when input transforms changed.
             */
            void recalculateInputRelatedData();

            void recalculateHeight();

            Signal<const ParticleMeasurer&> mDataChangedSignal; ///< Signal that emits after data has been recalculated.

            Slot<const SpatialOutput&> transformChangedSlot = { [&](const SpatialOutput&){ recalculate(); } };
            Slot<const glm::vec3&> vantagePointChangedSlot = { [&](const glm::vec3&){ recalculate(); } };
            Slot<const float&> groundLevelChangedSlot = { [&](const float& groundLevel){ recalculateHeight();             mDataChangedSignal.trigger(*this); } };

            void recalculate();

            float mDistanceToVantagePoint = 0.;
            std::vector<const SpatialTransform*> mInputTransforms;
            std::vector<float> mDistancesToInputs;
            float mHeight = 0.;

            Particle& mParticle;
            MeasurementComponentInstance& mMeasurementComponent;

        };


        /**
         * Component that measures various real-time data of the soundobject and its particles, like speed, angle, distance etc.
         * To be used by Effects.
         */
        class NAPAPI MeasurementComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(MeasurementComponent, MeasurementComponentInstance)

        public:
			MeasurementComponent() : Component() { }
			void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

        private:
        };


        /**
         * Instance of MeasurementComponent
         */
        class NAPAPI MeasurementComponentInstance : public ComponentInstance
        {
            friend class ParticleMeasurer;
            RTTI_ENABLE(ComponentInstance)

        public:
            MeasurementComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            Signal<const glm::vec3&>* getVantagePointChangedSignal() { return &mVantagePointChangedSignal; }

            Signal<const float&>* getGroundLevelChangedSignal() { return &mGroundLevelChangedSignal; }

            /**
             * Returns the particle measurer for a given index.
             */
            ParticleMeasurer* getParticleMeasurer(int index) { return mParticleMeasurers[index].get(); }

            /**
             * This is the active particle count including the particles that are already fading out.
             * So basically all particles that return isActive() true.
             */
            int getActiveParticleCount() const { return mActiveParticleCount; }

            /**
             * Called by EffectSoundComponent when a new input has been created.
             */
            void sourceCreated(SpatialSourceInstance& input);

        private:

            /**
             * Triggered when some input transform data changed. Makes all ParticleMeasurers recalculate their input related data.
             */
            Slot<const SpatialSourceInstance&> mInputDataChangedSlot = {[&](const auto& x){ recalculateInputRelatedData(); } };

            void recalculateInputRelatedData()
            {
                for(auto& particleMeasurer : mParticleMeasurers)
                    particleMeasurer->recalculateInputRelatedData();
            }

            Slot<SpatializationComponentInstance&, Particle&> mParticleCreatedSlot = {[&](SpatializationComponentInstance& component, Particle& particle)
            {
                particleCreated(component, particle);
            } };

            /**
             * Called when a new particle has been created.
             * Creates a new ParticleMeasurer for this particle.
             */
            void particleCreated(SpatializationComponentInstance& spatialSoundComponent, Particle& particle);

            Slot<Particle&> mParticleActivatedSlot = { this, &MeasurementComponentInstance::particleActivated };

            void particleActivated(Particle&);

            Signal<const glm::vec3&> mVantagePointChangedSignal;
            Signal<const float&> mGroundLevelChangedSignal;

            ParameterVec3* mVantagePoint = nullptr;
            ParameterFloat* mGroundLevel = nullptr;

            std::vector<std::unique_ptr<ParticleMeasurer>> mParticleMeasurers; // measurers of particles ordered by particle creation.

            // This is the active particle count including the particles that are already fading out.
            // So basically all particles that return isActive() true.
            // Atomic beause the particles deactivate on the audio thread.
            std::atomic<int> mActiveParticleCount = { 0 };
        };

    }
}
