#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainNode.h>


namespace nap
{
    
    namespace spatial
    {
    
        class DistanceIntensityEffectInstance;
        DECLARE_EFFECT(DistanceIntensityEffect, DistanceIntensityEffectInstance)

        class NAPAPI DistanceIntensityEffectProcessor : public ParallelNodeEffectProcessor<audio::FastGainNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)

        public:
            using ParallelNodeEffectProcessor<audio::FastGainNode>::ParallelNodeEffectProcessor;
        };
        
        
        /**
         * Distance intensity.
         */
        class NAPAPI DistanceIntensityEffectInstance : public SpatialEffectInstance<DistanceIntensityEffectProcessor> {
            RTTI_ENABLE(EffectInstanceBase)
        public:
            DistanceIntensityEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;

            virtual void recalculate(int processorIndex, int particleIndex) override;

            ParameterFloat* mDistanceIntensityThreshold = nullptr;
            ParameterFloat* mDistanceIntensityCurvature = nullptr;
            
        };
        
        
    }
    
}
