#pragma once

// Spatial audio includes
#include <Spatial/Audio/FastDelay.h>
#include <Spatial/Audio/FastGainNode.h>

// Audio includes
#include <audio/core/chain.h>
#include <Spatial/SpatialAudio/SpatialEffect.h>
#include <Spatial/SpatialAudio/DryWetEnableEffectProcessor.h>

// Nap includes
#include <nap/resource.h>

namespace nap
{

    namespace spatial
    {
        
        class SpatialDelayEffectInstance;
        DECLARE_EFFECT(SpatialDelayEffect, SpatialDelayEffectInstance)
        
        /**
         * SpatialDelayEffectProcessor: a delay chained to a gain.
         * Not optimal. The gain could be multiplied with an existing gain. 
         * But for now just porting the existing implementation.
         */
        class NAPAPI SpatialDelayEffectProcessor : public ParallelMonoEffectProcessor<audio::ChainInstance>
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
        public:
            using ParallelMonoEffectProcessor<audio::ChainInstance>::ParallelMonoEffectProcessor;

            bool initDSPChannel(int channel, audio::ChainInstance& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState) override;
        };

        
        
        /**
         * Spatial Delay.
         */
        class NAPAPI SpatialDelayEffectInstance : public SpatialEffectInstance<SpatialDelayEffectProcessor> {
            RTTI_ENABLE(EffectInstanceBase)
        public:
            SpatialDelayEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            
        private:
            virtual bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            
            virtual void update(double deltaTime) override;

            virtual void recalculate(int processorIndex, int index) override;
            
            virtual void onProcessorAdded(SpatialDelayEffectProcessor& processor) override;
            
            void recalculateRandomValues();
            
            void setGain(float gain);
            
            // parameters
            ParameterFloat* mDryWet = nullptr;
            ParameterFloat* mFeedback = nullptr;
            ParameterFloat* mDistanceScale = nullptr;
            ParameterFloat* mPeripheralScale = nullptr;
            ParameterFloat* mDopplerScale = nullptr;
            ParameterFloat* mRandomPatternScale = nullptr;
            ParameterBool* mRandomValueTrigger = nullptr;
            ParameterFloat* mNoiseSpeed = nullptr;
            ParameterFloat* mNoiseDepth = nullptr;
            ParameterFloat* mSmooth = nullptr;
            ParameterBool* mDopplerUnisono = nullptr;
            ParameterFloat* mGain = nullptr;
            
            ParameterFloat* mSpeedOfSound = nullptr; ///< The speed of sound in meter per seconds.
            
            // seeds for simplex noise per particle per processor
            std::vector<std::vector<float>> mSimplexSeeds;
            std::vector<std::vector<float>> mDelayTimeRandomValues;
            
            // accumulated time for simplex noise
            float simplexTimePassed = 0.;

            std::vector<std::vector<audio::SafePtr<audio::FastGainNode>>> mCachedGainNodes;
            std::vector<std::vector<audio::SafePtr<audio::FastDelayNode>>> mCachedDelayNodes;
            
            bool isTriggered = false; // switches to true after the random value trigger has been triggered. Switches back to false after it has been read.

        };
        
        
    }
    
}
