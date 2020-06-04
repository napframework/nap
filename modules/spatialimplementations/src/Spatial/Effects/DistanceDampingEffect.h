#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Audio includes
#include <Spatial/Audio/FilterChain.h>


namespace nap
{
    
    namespace spatial
    {
        
        class DistanceDampingEffectInstance;
        DECLARE_EFFECT(DistanceDampingEffect, DistanceDampingEffectInstance)        
        
        class NAPAPI DistanceDampingEffectProcessor : public ParallelMonoEffectProcessor<audio::FilterChainInstance>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
        public:
            using ParallelMonoEffectProcessor<audio::FilterChainInstance>::ParallelMonoEffectProcessor;

            bool initDSPChannel(int channel, audio::FilterChainInstance& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState) override
            {
                return channelInstance.init(nullptr, audio::FilterChainType::LowPass12dB, audioService.getNodeManager(), errorState);
            }
        };
        
        
        /**
         * Distance damping.
         */
        class NAPAPI DistanceDampingEffectInstance : public SpatialEffectInstance<DistanceDampingEffectProcessor> {
            RTTI_ENABLE(EffectInstanceBase)
        public:
            DistanceDampingEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            
            virtual void recalculate(int processorIndex, int index) override;
            
            ParameterFloat* mDistanceDampingThreshold = nullptr;
            ParameterFloat* mDistanceDampingCurvature = nullptr;
            
        };
        
        
    }
    
}
