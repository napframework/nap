#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/SpatialEffect.h>

// Audio includes
#include <Spatial/Audio/MonoTail.h>

// Nap includes
#include <nap/resource.h>


namespace nap
{

    namespace spatial
    {

        class ReverbEffectInstance;
        class DistanceDiffusionEffectInstance;
        
        DECLARE_EFFECT(ReverbEffect, ReverbEffectInstance)
        DECLARE_EFFECT(DistanceDiffusionEffect, DistanceDiffusionEffectInstance)

        class NAPAPI ReverbEffectProcessor : public ParallelNodeEffectProcessor<audio::MonoTailNode>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)

        public:
            using ParallelNodeEffectProcessor<audio::MonoTailNode>::ParallelNodeEffectProcessor;

        protected:
            void setChannelDefaults(int channel, audio::MonoTailNode& channelInstance) override
            {
                channelInstance.setCorrelationOffset((23 * channel) % 400); // correlation offset for diffusion between particles
                // An offset of 23 creates the most diffusion.
                // Correlation offsets greater than 400 cause unwelcome distortions.
            }

        };
        

        /**
         * A Reverb Effect with dry/wet parameter.
         */
        class NAPAPI ReverbEffectInstance : public SpatialEffectInstance<ReverbEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)

        public:
            ReverbEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}

        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;

            virtual void recalculate(int processorIndex, int particleIndex) override;

            ParameterFloat* mLength = nullptr;
            ParameterFloat* mDamping = nullptr;
            ParameterFloat* mDryWet = nullptr;
            ParameterFloat* mGain = nullptr;
            
            float mFeedbackValue = 0.;

        };
        
        
        /**
         * A Reverb Effect of which the dry/wet is controlled by the distance to the vantage point.
         */
        class NAPAPI DistanceDiffusionEffectInstance : public SpatialEffectInstance<ReverbEffectProcessor>
        {
            RTTI_ENABLE(EffectInstanceBase)
            
        public:
            DistanceDiffusionEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            
            virtual void recalculate(int processorIndex, int particleIndex) override;
            
            ParameterFloat* mLength = nullptr;
            ParameterFloat* mDamping = nullptr;
            ParameterFloat* mGain = nullptr;
            
            ParameterFloat* mDistanceThreshold = nullptr;
            ParameterFloat* mDistanceCurvature = nullptr;
            
            float mFeedbackValue = 0.;
            
        };

    }

}
