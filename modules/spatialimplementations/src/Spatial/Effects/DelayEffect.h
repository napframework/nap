#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Spatial Audio includes
#include <Spatial/Audio/FastDelay.h>

namespace nap
{
    namespace spatial
    {
        
        class DelayEffectInstance;
        
        
        /**
         * A simple delay effect with adjustable buffer size.
         */
        class NAPAPI DelayEffect : public Effect<DelayEffectInstance>
        {
            RTTI_ENABLE(EffectBase)
            
        public:
            DelayEffect() = default;
            
            int mBufferSize = 65536; ///< property: 'BufferSize' size of the delay buffer in samples
        };
        
        
        class NAPAPI DelayEffectProcessor : public ParallelNodeEffectProcessor<audio::FastDelayNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
            
        public:
            using ParallelNodeEffectProcessor<audio::FastDelayNode>::ParallelNodeEffectProcessor;

        private:
            void setChannelDefaults(int channel, audio::FastDelayNode& channelInstance) override {

                auto* resource = getResource<DelayEffect>();
                
                if(resource != nullptr)
                    channelInstance.setDelayLineSize(resource->mBufferSize);
                else
                    channelInstance.setDelayLineSize(65536);
            }

        };
        

        class NAPAPI DelayEffectInstance : public SpatialEffectInstance<DelayEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)
            
        public:
            DelayEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            virtual void recalculate(int processorIndex, int particleIndex) override;
            
            ParameterFloat* mDelayTime = nullptr;
        };
        
    }
}
