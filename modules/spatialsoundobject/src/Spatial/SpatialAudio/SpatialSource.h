#pragma once

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/ParameterManager.h>

// Audio includes
#include <audio/core/multichannel.h>


namespace nap
{

    // Forward declarations
    namespace audio
    {
        class AudioService;
    }
    

    namespace spatial
    {

        // Forward declarations
        class MixdownComponentInstance;
        class SpatialService;
        class RootProcess;
        class SpatialSourceInstance;

        /**
         * Resource for @SpatialSourceInstance.
         * SpatialSource resources need to override te instantiate() method to generate an instance from the resource's properties.
         */
        class NAPAPI SpatialSource : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            /**
             * Generate an instance of this resource
             * @param service the spatial service, used for access to ADC inputs and the audio service
             * @param particleCount maximum number of particles
             * @param parameterComponent parameter component of the sound object
             * @param errorState contains error logging
             * @return new instance of SpatialSourceInstance
             */
            virtual std::unique_ptr<SpatialSourceInstance> instantiate(SpatialService& service, int particleCount, ParameterComponentInstance& parameterComponent, utility::ErrorState& errorState) = 0;

        private:
        };
        
        /**
         * Instance of @SpatialSource
         * A SpatialSourceInstance is a class that generates a particlecount-channel output with and an 'inputtransform' per particle (which should indicate the transform of the source of the signal for this particle.)
         * It functions as an input for the InputEffectChain.
         */
        class NAPAPI SpatialSourceInstance : public audio::IMultiChannelOutput {
            RTTI_ENABLE()

        public:
            SpatialSourceInstance(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& namePrefix);

            virtual ~SpatialSourceInstance() = default;

            /**
             * Returns the input transform for a specific particle.
             */
            virtual const SpatialTransform* getInputTransformForChannel(int channel) const = 0;

            /**
             * Called by the SpatialAudioComponent whenever the number of active particles changes.
             */
            virtual void activeParticleCountChanged(int) { }

            /**
             * IMultiChannelOutput function. Returns the particle count.
             */
            int getChannelCount() const override { return mParticleCount; };

            /**
             * Returns a signal that triggers when one or more input transforms have been changed.
             */
            Signal<const SpatialSourceInstance&>* getDataChangedSignal() { return &mDataChangedSignal; }

            const std::string& getNamePrefix() const { return mParameterManager.getNamePrefix(); }

        protected:
            ParameterManager& getParameterManager() { return mParameterManager; }
            const ParameterManager& getParameterManager() const { return mParameterManager; }
            SpatialService& getSpatialService() { return mSpatialService; }

        private:
            int mParticleCount = 0;
            
            Signal<const SpatialSourceInstance&> mDataChangedSignal; // signal that should trigger when an input transform has changed..

            ParameterManager mParameterManager;
            SpatialService& mSpatialService;
        };


    }
}

