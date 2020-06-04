#pragma once

// Spatial include
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Audio include
#include <Spatial/Audio/FastGainNode.h>


namespace nap
{
    
    namespace spatial
    {
        
        class InputDistanceIntensityEffectInstance;
        DECLARE_EFFECT(InputDistanceIntensityEffect, InputDistanceIntensityEffectInstance)
        
        
        class NAPAPI InputDistanceIntensityEffectProcessor : public ParallelNodeEffectProcessor<audio::FastGainNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)

        public:
            using ParallelNodeEffectProcessor<audio::FastGainNode>::ParallelNodeEffectProcessor;
        };
        
        
        /**
         * Distance intensity.
         */
        class NAPAPI InputDistanceIntensityEffectInstance : public SpatialEffectInstance<InputDistanceIntensityEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)
            
        public:
            InputDistanceIntensityEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            
            virtual void recalculate(int processorIndex, int particleIndex) override;
            
            ParameterFloat* mInputDistanceIntensityThreshold = nullptr;
            ParameterFloat* mInputDistanceIntensityCurvature = nullptr;
            
        };
        

    }
    
}
