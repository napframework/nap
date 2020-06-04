#pragma once

#include <Spatial/SpatialAudio/SpatialSource.h>
#include <audio/resource/audiofileresource.h>
#include <audio/utility/translator.h>
#include <audio/utility/dirtyflag.h>

namespace nap
{

    // Forward declarations
    namespace audio
    {
        class GranulatorInstance;
    }

    namespace spatial
    {


        class NAPAPI GranulatorSource : public SpatialSource
        {
            RTTI_ENABLE(SpatialSource)

        public:
            std::string mName = ""; ///< property: 'Name' Prefix name for parameters
            int mInternalBufferSize = 32; ///< property: 'InternalBufferSize' size of the internal buffer for audio processing in samples. This indicates the precision of the timing of grains.
            int mVoiceCount = 200; ///< property: 'VoiceCount' number of voices the granulator manages for grain playback.
            std::vector<std::string> mAudioFiles; ///< property: 'AudioFiles' paths to audio files to use as sample material

            // Inherited SpatialSource
            std::unique_ptr<SpatialSourceInstance> instantiate(SpatialService& service, int particleCount, ParameterComponentInstance& parameterComponent, utility::ErrorState& errorState) override;

        private:
        };


        class NAPAPI GranulatorSourceInstance : public SpatialSourceInstance
        {
            RTTI_ENABLE(SpatialSourceInstance)

        public:
            GranulatorSourceInstance(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& namePrefix) : SpatialSourceInstance(service, channelCount, parameterComponent, namePrefix) { }

            bool init(int internalBufferSize, int voiceCount, const std::vector<std::string>& audioFiles, utility::ErrorState& errorState);

            audio::OutputPin* getOutputForChannel(int channel) override;
            const SpatialTransform* getInputTransformForChannel(int channel) const override { return &mTransform; }
            void activeParticleCountChanged(int) override;

        private:
            // Called from the audio thread every buffer cycle
            Slot<audio::DiscreteTimeValue> mUpdateAudioSlot = { this, &GranulatorSourceInstance::updateAudio };
            void updateAudio(audio::DiscreteTimeValue sampleTime);

            std::unique_ptr<audio::GranulatorInstance> mGranulator = nullptr;
            audio::SafeOwner<audio::TableTranslator<float>> mPitchTranslator = nullptr;
            std::vector<std::unique_ptr<audio::AudioFileResource>> mAudioFiles;

            audio::DirtyFlag mIsDirty;
            std::atomic<float> mDuration = { 0 };
            std::atomic<float> mDensity =  { 0 };

            SpatialTransform mTransform; // Just one default transform to pass as the input transform (as external inputs don't have a transform).
        };

    }

}