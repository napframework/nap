#pragma once

// Spatial Audio include
#include <Spatial/Audio/FastGainNode.h>

// Spatial include
#include <Spatial/SpatialAudio/SpatialEffect.h>


namespace nap
{
    
    namespace spatial
    {
        
        class GainScalingEffectInstance;
        DECLARE_EFFECT(GainScalingEffect, GainScalingEffectInstance)
        
        class NAPAPI GainScalingEffectProcessor : public ParallelNodeEffectProcessor<audio::FastGainNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)

        public:
            using ParallelNodeEffectProcessor<audio::FastGainNode>::ParallelNodeEffectProcessor;
        };
        
        
        /**
         * An Effect that decreases the volume of the particles when the particle count increases.
         */
        class NAPAPI GainScalingEffectInstance : public SpatialEffectInstance<GainScalingEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)
            
        public:
            GainScalingEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            virtual void update(double deltaTime) override;
            virtual void recalculate(int processorIndex, int particleIndex) override;
            
            void recalculateGain();
            
            int mParticleCount = 1;
            
            ParameterFloat* mAmount = nullptr;
            float mGain = 1.;
            
        };
        
    }
    
}
