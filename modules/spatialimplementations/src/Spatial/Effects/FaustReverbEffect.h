#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Audio includes
#include <Spatial/Audio/FreeverbNode.h>


namespace nap
{

    namespace spatial
    {

        class FaustReverbEffectInstance;
        DECLARE_EFFECT(FaustReverbEffect, FaustReverbEffectInstance)

        class NAPAPI FaustReverbEffectProcessor : public ParallelNodeEffectProcessor<audio::FreeverbNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)

        public:
            using ParallelNodeEffectProcessor<audio::FreeverbNode>::ParallelNodeEffectProcessor;

        protected:
            void setChannelDefaults(int channel, audio::FreeverbNode& channelInstance) override
            {
                channelInstance.setSpread(channel / float(getChannelCount()) * 100.f);
            }

        };


        /**
         * An Effect that decreases the volume of the particles when the particle count increases.
         */
        class NAPAPI FaustReverbEffectInstance : public SpatialEffectInstance<FaustReverbEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)

        public:
            FaustReverbEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}

        protected:
            virtual void onProcessorAdded(FaustReverbEffectProcessor& processor) override;
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            void update(double deltaTime) override;

            virtual void recalculate(int processorIndex, int particleIndex) override { }

            ParameterFloat* mFeedback = nullptr;
            ParameterFloat* mFeedback2 = nullptr;
            ParameterFloat* mDamping = nullptr;
            ParameterFloat* mDryWet = nullptr;

            int mParticleCount = 0;
        };

    }

}
