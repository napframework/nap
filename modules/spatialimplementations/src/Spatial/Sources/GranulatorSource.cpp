#include "GranulatorSource.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <Spatial/Audio/Granulator.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/core/nestednodemanager.h>
#include <audio/core/polyphonic.h>


RTTI_BEGIN_CLASS(nap::spatial::GranulatorSource)
    RTTI_PROPERTY("Name", &nap::spatial::GranulatorSource::mName, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("InternalBufferSize", &nap::spatial::GranulatorSource::mInternalBufferSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceCount", &nap::spatial::GranulatorSource::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AudioFiles", &nap::spatial::GranulatorSource::mAudioFiles, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GranulatorSourceInstance)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        std::unique_ptr<SpatialSourceInstance> GranulatorSource::instantiate(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, utility::ErrorState& errorState)
        {
            auto result = std::make_unique<GranulatorSourceInstance>(service, channelCount, parameterComponent, mName);
            if (!result->init(mInternalBufferSize, mVoiceCount, mAudioFiles, errorState))
            {
                errorState.fail("Failed to initialize GranulatorSource: %s", errorState.toString().c_str());
                return nullptr;
            }
            return std::move(result);
        }


        bool GranulatorSourceInstance::init(int internalBufferSize, int voiceCount, const std::vector<std::string>& audioFiles, utility::ErrorState& errorState)
        {
            auto& nodeManager = getSpatialService().getAudioService().getNodeManager();

            mPitchTranslator = nodeManager.makeSafe<audio::TableTranslator<float>>(2048);
            mPitchTranslator->fill([](float input)
                                   {
                                       return std::pow(2, (-12 + input * 24) / 12.f);
                                   });

            if (audioFiles.empty())
            {
                errorState.fail("No audiofiles provided for GranulatorSource");
                return false;
            }
            for (auto& path : audioFiles)
            {
                auto buffer = std::make_unique<audio::AudioFileResource>(getSpatialService().getAudioService());
                buffer->mAudioFilePath = path;
                if (!buffer->init(errorState))
                {
                    errorState.fail("Failed to load audio file: %s", path.c_str());
                    return false;
                }
                mAudioFiles.emplace_back(std::move(buffer));
            }

            mGranulator = std::make_unique<audio::GranulatorInstance>();
            if (!mGranulator->init(getChannelCount(), internalBufferSize, voiceCount, mAudioFiles.front()->getBuffer(), mPitchTranslator.get(), nodeManager, errorState))
            {
                errorState.fail("Failed to initialize GranulatorInstance");
                return false;
            }

            auto granulatorBufferIndex = getParameterManager().addParameterInt("buffer", 0, 0, mAudioFiles.size() - 1);
//            granulatorDensity->setEditorCurve(4);
            granulatorBufferIndex->connect([&](int value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setSource(mAudioFiles[value]->getBuffer());
            });

            auto granulatorDensity = getParameterManager().addParameterFloat("density", 1.f, 0.005, 5.f);
//            granulatorDensity->setEditorCurve(4);
            granulatorDensity->connect([&](float value){
                mDensity.store(value);
                mIsDirty.set();
            });

            auto granulatorDuration = getParameterManager().addParameterFloat("duration", 50.f, 5.f, 5000.f);
//            granulatorDuration->setEditorCurve(4);
            granulatorDuration->connect([&](float value){
                mDuration.store(value);
                mIsDirty.set();
            });

            auto bufferLength = mAudioFiles.front()->toMilliseconds(mAudioFiles.front()->getSize());
            auto granulatorPosition = getParameterManager().addParameterFloat("position", 0.f, 0.f, bufferLength);
//            granulatorDuration->setEditorCurve(4);
            granulatorPosition->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setPosition(value);
            });

            auto granulatorDiffusion = getParameterManager().addParameterFloat("diffusion", 10.f, 1.f, 2000.f);
//            granulatorDiffusion->setEditorCurve(2.25);
            granulatorDiffusion->connect([&](float value)
            {
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setDiffusion(value);
            });

            auto granulatorIrregularity = getParameterManager().addParameterFloat("timeDeviation", 0.5f, 0.f, 1.f);
            granulatorIrregularity->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setIrregularity(value);
            });

            auto granulatorAmplitude = getParameterManager().addParameterFloat("amplitude", 1.f, 0.f, 2.f);
            granulatorAmplitude->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setAmplitude(value);
            });

            auto granulatorAmplitudeDev = getParameterManager().addParameterFloat("amplitudeDeviation", 0.f, 0.f, 1.f);
            granulatorAmplitudeDev->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setAmplitudeDev(value);
            });

            auto granulatorPitch = getParameterManager().addParameterFloat("transpose", 0.f, -12.f, 12.f);
            granulatorPitch->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setTranspose(value);
            });

            auto granulatorDetune = getParameterManager().addParameterFloat("transposeDeviation", 0.f, 0.f, 12.f);
            granulatorDetune->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setTransposeDeviation(value);
            });

            auto granulatorAttackDecay = getParameterManager().addParameterFloat("slope", 0.f, 0.f, 100.f);
//            granulatorAttackDecay->setEditorCurve(2);
            granulatorAttackDecay->connect([&](float value){
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setAttackDecay(value);
            });

            auto granulatorShape = getParameterManager().addParameterOptionList("shape", "hanning", { "hanning", "expodec", "rexpodec" });
            granulatorShape->connect([&](int index) {
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    mGranulator->getChannel(channel).setShape(audio::GranulatorInstance::Shape(index));
            });

            nodeManager.mUpdateSignal.connect(mUpdateAudioSlot);

            mGranulator->setActive(true);

            return true;
        }


        audio::OutputPin* GranulatorSourceInstance::getOutputForChannel(int channel)
        {
            return mGranulator->getOutputForChannel(channel);
        }


        void GranulatorSourceInstance::activeParticleCountChanged(int count)
        {
            mGranulator->setActiveChannelCount(count);
        }


        void GranulatorSourceInstance::updateAudio(audio::DiscreteTimeValue sampleTime)
        {
            if (mIsDirty.check())
            {
                auto duration = mDuration.load();
                auto density = mDensity.load();
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                {
                    mGranulator->getChannel(channel).setDuration(duration);
                    mGranulator->getChannel(channel).setDensity(density);
                }
            }
        }



    }

}
