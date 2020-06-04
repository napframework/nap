#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/MeasurementComponent.h>
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/SpatialAudio/Effect.h>
#include <Spatial/SpatialAudio/DryWetEnableEffectProcessor.h>
#include <Spatial/Transformation/SpatialTransformationComponent.h>
#include <Spatial/Utility/ParameterTypes.h>

// Audio includes
#include <audio/core/graphobject.h>

// Std includes
#include <stdio.h>

namespace nap
{

    namespace spatial
    {

        /**
         * SpatialEffectInstance is an EffectInstance that implements an enable parameter and has a getParticleMeasurer() function to access particle data.
         * It is the base class for sound object effects.
         * SpatialEffectInstances can work with any @DryWetEnableEffectProcessor type.
         */
        template <typename ProcessorType>
        class NAPAPI SpatialEffectInstance : public EffectInstance<ProcessorType>
        {
            RTTI_ENABLE(EffectInstanceBase)

        public:
            SpatialEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : EffectInstance<ProcessorType>(effect, audioService, channelCount) { }

            bool init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr) override final;

        protected:
            /**
             * Virtual function to be implemented by derived class, called from init().
             * Initialise parameters and connect signals to recalculate functions here.
             */
            virtual bool onInit(nap::EntityInstance *entity, utility::ErrorState &errorState) { return true; }

            /**
             * Convenience function that ensures recalculateParticle(i) will get called if the data of a particle has changed.
             */
            void recalculateParticleOnDataChange();
            
            
            /**
             * Function that ensures all particles will recalculate when a parameter has changed.
             * Note: has to be implement separately for each parameter type, because there is no generic valueChanged signal in the Parameter base class.
             */
            void recalculateOnChange(ParameterFloat* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            void recalculateOnChange(ParameterInt* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            void recalculateOnChange(ParameterBool* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            void recalculateOnChange(ParameterString* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            void recalculateOnChange(ParameterOptionList* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            void recalculateOnChange(ParameterVec3* parameter)
            {
                parameter->connect([&](const auto& x){ recalculateAllParticles(); });
            }
            
            
            /**
             * Recalculates and sets the DSP parameters for 1 particle.
             */
            virtual void recalculate(int processorIndex, int particleIndex) = 0;

            /**
             * Recalculates and sets the DSP parameters for 1 particle for every processor.
             */
            void recalculateParticle(int particleIndex);

            /**
             * Recalculates and sets the DSP parameters of all particles for one processor.
             */
            void recalculateProcessor(int processorIndex);

            /**
             * Recalculates all particles for all processors.
             */
            void recalculateAllParticles();

            /**
             * Returns the particle measure for the given particleIndex.
             */
            ParticleMeasurer* getParticleMeasurer(int particleIndex);

            /**
             * Returns the number of particles that are currently active.
             */
            int getActiveParticleCount() const { return mMeasurementComponent->getActiveParticleCount(); }
            
            /**
             * Convenience function.
             */
            Signal<const ParticleMeasurer&>* getParticleDataChangedSignal(int index);

            /**
             * Adjusts the dry/wet balance of the processor.
             */
            void setDryWet(float dryWet);
            
            /**
             * Separately adjusts the dry level of the processor.
             */
            void setDry(float dry);

            /**
             * Separately adjusts the wet level of the processor.
             */
            void setWet(float wet);
            
            /**
             * Returns whether this effect is enabled or not.
             */
            bool isEnabled()
            {
                return mEnable->mValue;
            }

            SpatialTransform& getSoundObjectTransform(){ return mSpatialTransformationComponent->getSoundObjectTransform(); }

            ParameterManager& getParameterManager() { return mParameterManager; };

            
        protected:
            /**
             * Called whenever the effect is enabled.
             */
            virtual void onEnable() { }
            
            /**
             * Called whenever the effect is disabled.
             */
            virtual void onDisable() { }
            
            /**
             * Virtual function that is called directly after a processor has been added.
             * Overriden by default to call recalculateProcessor() so the processor values are up-to-date with the parameter values.
             */
            virtual void onProcessorAdded(ProcessorType& newProcessor) override
            {
                recalculateProcessor(EffectInstance<ProcessorType>::getProcessorCount() - 1);
            }
            
        private:
            /**
             * Enables/bypasses the processor.
             */
            void setEnable(bool enable);

            ParameterBool* mEnable = nullptr;
            ParameterManager mParameterManager;
            MeasurementComponentInstance* mMeasurementComponent = nullptr;
            SpatialTransformationComponentInstance* mSpatialTransformationComponent = nullptr; // to get the sound object transform
        };


        template <typename ProcessorType>
        bool SpatialEffectInstance<ProcessorType>::init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity) {

            if(!entity)
                return false;

            // init parameter manager
            mParameterManager.init(entity->getComponent<ParameterComponentInstance>(), "effect/" + EffectInstance<ProcessorType>::getName(), "effect/shared");

            // init enable
            mEnable = mParameterManager.addParameterBool("enable", false);

            // connect enable parameter to setEnable function
            mEnable->valueChanged.connect([&](bool value){
                setEnable(value);
            });

            // get measurement component
            mMeasurementComponent = &entity->getComponent<MeasurementComponentInstance>();
            if (mMeasurementComponent == nullptr)
            {
                errorState.fail("Unable to find MeasurementComponent");
                return false;
            }
            
            mSpatialTransformationComponent = &entity->getComponent<SpatialTransformationComponentInstance>();
            if (mSpatialTransformationComponent == nullptr)
            {
                errorState.fail("Unable to find SpatialTransformationComponent");
                return false;
            }

            // init custom parameters
            if (!onInit(entity, errorState))
                return false;

            return true;
        }


        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::recalculateParticleOnDataChange()
        {
            // connect particle data changed signals to recalculateParticle(i) function
            for(int i = 0; i < EffectInstance<ProcessorType>::getChannelCount(); i++)
                getParticleDataChangedSignal(i)->connect([&, i](const auto& x){ recalculateParticle(i); });

        }


        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::recalculateAllParticles()
        {
            for(int j = 0; j < EffectInstance<ProcessorType>::getProcessorCount(); j++){
                for(int i = 0; i < EffectInstance<ProcessorType>::getChannelCount(); i++){
                    recalculate(j, i);
                }
            }
        }

        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::recalculateParticle(int particleIndex)
        {
            for(int j = 0; j < EffectInstance<ProcessorType>::getProcessorCount(); j++){
                recalculate(j, particleIndex);
            }
        }


        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::recalculateProcessor(int processorIndex)
        {
            for(int i = 0; i < EffectInstance<ProcessorType>::getChannelCount(); i++){
                recalculate(processorIndex, i);
            }
        }


        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::setEnable(bool enable)
        {
            for(int i = 0; i < EffectInstance<ProcessorType>::getProcessorCount(); i++)
            {
                auto processor = EffectInstance<ProcessorType>::getProcessor(i);
                processor->setEnable(enable);
            }
            if (enable)
                onEnable();
            else
                onDisable();
        }


        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::setDryWet(float value)
        {
            for(int i = 0; i < EffectInstance<ProcessorType>::getProcessorCount(); i++)
            {
                auto processor = EffectInstance<ProcessorType>::getProcessor(i);
                processor->setDryWet(value);
            }
        }
        
        
        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::setDry(float value)
        {
            for(int i = 0; i < EffectInstance<ProcessorType>::getProcessorCount(); i++)
            {
                auto processor = EffectInstance<ProcessorType>::getProcessor(i);
                processor->setDry(value);
            }
        }

        
        template <typename ProcessorType>
        void SpatialEffectInstance<ProcessorType>::setWet(float value)
        {
            for(int i = 0; i < EffectInstance<ProcessorType>::getProcessorCount(); i++)
            {
                auto processor = EffectInstance<ProcessorType>::getProcessor(i);
                processor->setWet(value);
            }
        }


        template <typename ProcessorType>
        ParticleMeasurer* SpatialEffectInstance<ProcessorType>::getParticleMeasurer(int particleIndex)
        {
            return mMeasurementComponent->getParticleMeasurer(particleIndex);
        }


        template <typename ProcessorType>
        Signal<const ParticleMeasurer&>* SpatialEffectInstance<ProcessorType>::getParticleDataChangedSignal(int index){
            return getParticleMeasurer(index)->getDataChangedSignal();
        }

    }
}
