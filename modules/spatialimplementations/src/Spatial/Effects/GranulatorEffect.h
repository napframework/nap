#pragma once

// Spatial audio includes
#include <Spatial/SpatialAudio/SpatialEffect.h>
#include <Spatial/SpatialAudio/DryWetEnableEffectProcessor.h>
#include <Spatial/Audio/Granulator.h>

// Audio includes
#include <audio/utility/dirtyflag.h>


namespace nap
{
    // forward declares
    namespace audio
    {
        class CircularBufferInstance;
    }
    
    namespace spatial
    {
        class RootProcess;
        
        class GranulatorEffectInstance;
        

        /**
         * Effect resource for the granulator effect.
         */
        class NAPAPI GranulatorEffect : public Effect<GranulatorEffectInstance>
        {
            RTTI_ENABLE(EffectBase)

        public:
            GranulatorEffect() = default;

            int mCircularBufferSize = 65536 * 8; ///< property: 'CircularBufferSize' size of the circular buffer in samples
            int mInternalBufferSize = 32; ///< property: 'InternalBufferSize' size of the internal buffer for audio processing in samples. This indicates the precision of the timing of grains.
            int mVoiceCount = 200; ///< property: 'VoiceCount' number of voices the granulator manages for grain playback.
        };



        class NAPAPI GranulatorEffectProcessor : public DryWetEnableEffectProcessor
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
            friend class GranulatorEffectInstance;

        public:
            using DryWetEnableEffectProcessor::DryWetEnableEffectProcessor;

            // Inherited from DryWetEnableEffectProcessor

            // Creates and returns a @GranulatorInstance
            std::unique_ptr<audio::AudioObjectInstance> createDSP(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState) override;

            // The granulator (altough it processes multiple particles) practically has only one input channel right now. So this function only performs something if @channel is zero. It connects an audio pin to the circular buffer writer.
            void connectToDSP(unsigned int channel, audio::OutputPin& pin) override;

            /**
             * @return the multichannel granulator audio object instance. Every channel contains a mono granulator algorithm.
             */
            audio::GranulatorInstance& getGranulator() { return *mGranulator; }

        private:
            audio::CircularBufferNode& getCircularBuffer();

            // Circular buffer that the audio material in the grains is sampled from
            std::unique_ptr<audio::CircularBufferInstance> mCircularBuffer = nullptr;

            // Table used to convert semitones to pitch
            audio::SafeOwner<audio::TableTranslator<float>> mPitchTranslator;
            audio::GranulatorInstance* mGranulator = nullptr;
        };
        
        
        /**
         * Instance of the granulator effect.
         */
        class NAPAPI GranulatorEffectInstance : public SpatialEffectInstance<GranulatorEffectProcessor> {
            RTTI_ENABLE(EffectInstanceBase)

        public:
            GranulatorEffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : SpatialEffectInstance(effect, audioService, channelCount) {}
            ~GranulatorEffectInstance() override;

            // Inherited from SpatialEffectInstance
            void update(double deltaTime) override;

        private:
            // Inherited from SpatialEffectInstance
            bool onInit(EntityInstance* entity, utility::ErrorState& errorState) override;
            void onEnable() override;
            void onDisable() override;
            void onProcessorAdded(GranulatorEffectProcessor& newProcessor) override;

            // Called from the audio thread every buffer cycle
            Slot<audio::DiscreteTimeValue> mUpdateAudioSlot = { this, &GranulatorEffectInstance::updateAudio };
            void updateAudio(audio::DiscreteTimeValue sampleTime);

        private:
            // Inherited from SpatialEffectInstance
            void recalculate(int processorIndex, int index) override;
            
            RootProcess* mRootProcess = nullptr;
            int mActiveChannelCount = 0;
            audio::DirtyFlag mIsDirty;
            std::atomic<float> mDuration = { 0 };
            std::atomic<float> mDensity =  { 0 };
            ParameterFloat* mDryWet = nullptr;
            ParameterFloat* mGain = nullptr;
            
            ParameterFloat* mGranulatorDensity = nullptr;
            ParameterFloat* mGranulatorDuration = nullptr;
            ParameterFloat* mGranulatorDiffusion = nullptr;
            ParameterFloat* mGranulatorIrregularity = nullptr;
            ParameterFloat* mGranulatorAmplitude = nullptr;
            ParameterFloat* mGranulatorAmplitudeDev = nullptr;
            ParameterFloat* mGranulatorPitch = nullptr;
            ParameterFloat* mGranulatorDetune = nullptr;
            ParameterFloat* mGranulatorAttackDecay = nullptr;
            ParameterOptionList* mGranulatorShape = nullptr;
        };
        
        
    }
    
}
