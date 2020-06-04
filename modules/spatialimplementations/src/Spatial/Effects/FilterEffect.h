#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Audio includes
#include <Spatial/Audio/FilterChain.h>


namespace nap
{
    
    namespace spatial
    {
        
        class FilterEffectInstance;
        
        /**
         * A Filter per particle.
         */
        class NAPAPI FilterEffect : public Effect<FilterEffectInstance>
        {
            RTTI_ENABLE(EffectBase)
            
        public:
            FilterEffect() = default;
            
            audio::FilterChainType mFilterChainType = audio::FilterChainType::LowPass6dB;
        };
        
        
        class NAPAPI FilterEffectProcessor : public ParallelMonoEffectProcessor<audio::FilterChainInstance>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
        public:
            using ParallelMonoEffectProcessor<audio::FilterChainInstance>::ParallelMonoEffectProcessor;
            
            bool initDSPChannel(int channel, audio::FilterChainInstance& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState) override
            {
                auto resource = getResource<FilterEffect>();
                return channelInstance.init(nullptr, resource->mFilterChainType, audioService.getNodeManager(), errorState);
            }
        };
        
        class NAPAPI FilterEffectInstance : public SpatialEffectInstance<FilterEffectProcessor> {
            RTTI_ENABLE(EffectInstanceBase)
        public:
            FilterEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override
            {
                float defaultValue = 20.;
                auto filterChainType = getResource<FilterEffect>()->mFilterChainType;
                if(filterChainType == audio::FilterChainType::LowPass6dB ||
                   filterChainType == audio::FilterChainType::LowPass12dB ||
                   filterChainType == audio::FilterChainType::LowPass18dB ||
                   filterChainType == audio::FilterChainType::LowPass24dB)
                {
                    defaultValue = 22000.;
                }
                
                mFrequency = getParameterManager().addParameterFloat("frequency", defaultValue, 20, 22000);
                recalculateOnChange(mFrequency);
                return true;
            }
            
            virtual void recalculate(int processorIndex, int particleIndex) override
            {
                getProcessor(processorIndex)->getDSP(particleIndex)->setFrequency(mFrequency->mValue);
            }
            
            ParameterFloat* mFrequency = nullptr;
            
        };
        
    }
    
}
