#include "GranulatorEffect.h"

// Spatial includes
#include <Spatial/Utility/AudioFunctions.h>
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/RootProcess.h>

// Audio includes
#include <audio/utility/audiofunctions.h>
#include <audio/core/nestednodemanager.h>
#include <audio/resource/equalpowertable.h>
#include <audio/object/circularbuffer.h>
#include <audio/core/polyphonic.h>



RTTI_BEGIN_CLASS(nap::spatial::GranulatorEffect)
    RTTI_PROPERTY("CircularBufferSize", &nap::spatial::GranulatorEffect::mCircularBufferSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("InternalBufferSize", &nap::spatial::GranulatorEffect::mInternalBufferSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceCount", &nap::spatial::GranulatorEffect::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GranulatorEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GranulatorEffectInstance)
RTTI_END_CLASS


namespace nap
{
    
    namespace spatial
    {

        std::unique_ptr<audio::AudioObjectInstance> GranulatorEffectProcessor::createDSP(audio::AudioService &audioService, int channelCount, utility::ErrorState &errorState)
        {
            auto resource = getResource<GranulatorEffect>();

            mCircularBuffer = std::make_unique<audio::CircularBufferInstance>();
            if (!mCircularBuffer->init(1, false, resource->mCircularBufferSize, audioService.getNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize CircularBuffer");
                return nullptr;
            }

            mPitchTranslator = audioService.getNodeManager().makeSafe<audio::TableTranslator<float>>(2048);
            mPitchTranslator->fill([](float input)
            {
                return std::pow(2, (-12 + input * 24) / 12.f);
            });

            auto granulatorInstance = std::make_unique<audio::GranulatorInstance>();
            if (!granulatorInstance->init(channelCount, resource->mInternalBufferSize, resource->mVoiceCount, *mCircularBuffer, mPitchTranslator.get(), audioService.getNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize granulator");
                return nullptr;
            }
            mGranulator = granulatorInstance.get();

            return std::move(granulatorInstance);
        }


        void GranulatorEffectProcessor::connectToDSP(unsigned int channel, audio::OutputPin &pin)
        {
            if (channel == 0)
                mCircularBuffer->getChannel(0)->audioInput.connect(pin);
        }

        
        audio::CircularBufferNode& GranulatorEffectProcessor::getCircularBuffer()
        {
            return *mCircularBuffer->getChannel(0);
        }


        GranulatorEffectInstance::~GranulatorEffectInstance()
        {
            for (auto i = 0; i < getProcessorCount(); ++i)
                mRootProcess->unregisterPostParticleProcess(getProcessor(i)->getCircularBuffer());
        }


        bool GranulatorEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            // get spatial sound component component
            auto spatialSoundComponent = &entity->getComponent<SpatializationComponentInstance>();
            if (spatialSoundComponent == nullptr)
            {
                errorState.fail("Unable to find SpatializationComponent");
                return false;
            }
            mRootProcess = &spatialSoundComponent->getSpatialService().getRootProcess();

            mDryWet = getParameterManager().addParameterFloat("dryWet", 0.f, 0.f, 1.f);
            mDryWet->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                {
                    getProcessor(i)->setDry(1.f - value);
                    getProcessor(i)->setWet(value * audio::dbToA(mGain->mValue));
                }
            });

            mGain = getParameterManager().addParameterFloat("gain", 0.0, 0.0, 24.0);
            mGain->connect([&](float value)
            {
                for (auto i = 0; i < getProcessorCount(); ++i)
                    getProcessor(i)->setWet(audio::dbToA(mGain->mValue) * mDryWet->mValue);
            });

            mGranulatorDensity = getParameterManager().addParameterFloat("density", 1.f, 0.005, 5.f);
//            granulatorDensity->setEditorCurve(4);
            mGranulatorDensity->connect([&](float value){
                mDensity.store(value);
                mIsDirty.set();
            });

            mGranulatorDuration = getParameterManager().addParameterFloat("duration", 50.f, 5.f, 5000.f);
//            granulatorDuration->setEditorCurve(4);
            mGranulatorDuration->connect([&](float value){
                mDuration.store(value);
                mIsDirty.set();
            });

            mGranulatorDiffusion = getParameterManager().addParameterFloat("diffusion", 10.f, 1.f, 2000.f);
//            granulatorDiffusion->setEditorCurve(2.25);
            mGranulatorDiffusion->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setDiffusion(value);
            });

            mGranulatorIrregularity = getParameterManager().addParameterFloat("timeDeviation", 0.5f, 0.f, 1.f);
            mGranulatorIrregularity->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setIrregularity(value);
            });

            mGranulatorAmplitude = getParameterManager().addParameterFloat("amplitude", 1.f, 0.f, 2.f);
            mGranulatorAmplitude->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setAmplitude(value);
            });

            mGranulatorAmplitudeDev = getParameterManager().addParameterFloat("amplitudeDeviation", 0.f, 0.f, 1.f);
            mGranulatorAmplitudeDev->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setAmplitudeDev(value);
            });

            mGranulatorPitch = getParameterManager().addParameterFloat("transpose", 0.f, -12.f, 12.f);
            mGranulatorPitch->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setTranspose(value);
            });

            mGranulatorDetune = getParameterManager().addParameterFloat("transposeDeviation", 0.f, 0.f, 12.f);
            mGranulatorDetune->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setTransposeDeviation(value);
            });

            mGranulatorAttackDecay = getParameterManager().addParameterFloat("slope", 0.f, 0.f, 100.f);
//            granulatorAttackDecay->setEditorCurve(2);
            mGranulatorAttackDecay->connect([&](float value){
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setAttackDecay(value);
            });

            mGranulatorShape = getParameterManager().addParameterOptionList("shape", "hanning", { "hanning", "expodec", "rexpodec" });
            mGranulatorShape->connect([&](int index)
            {
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                        getProcessor(i)->getGranulator().getChannel(channel).setShape(audio::GranulatorInstance::Shape(index));
            });


            getAudioService().getNodeManager().mUpdateSignal.connect(mUpdateAudioSlot);

            return true;
        }


        void GranulatorEffectInstance::recalculate(int processorIndex, int index)
        {

        }


        void GranulatorEffectInstance::update(double deltaTime)
        {
            auto activeParticleCount = getActiveParticleCount();
            if (activeParticleCount == mActiveChannelCount)
                return;
            mActiveChannelCount = activeParticleCount;
            for (auto i = 0; i < getProcessorCount(); ++i)
            {
                auto processor = getProcessor(i);
                processor->getGranulator().setActiveChannelCount(mActiveChannelCount);
            }
        }


        void GranulatorEffectInstance::updateAudio(audio::DiscreteTimeValue sampleTime)
        {
            if (mIsDirty.check())
            {
                auto duration = mDuration.load();
                auto density = mDensity.load();
                for (auto i = 0; i < getProcessorCount(); ++i)
                    for (auto channel = 0; channel < getProcessor(i)->getGranulator().getChannelCount(); ++channel)
                    {
                        getProcessor(i)->getGranulator().getChannel(channel).setDuration(duration);
                        getProcessor(i)->getGranulator().getChannel(channel).setDensity(density);
                    }
            }
        }



        void GranulatorEffectInstance::onDisable()
        {
            for (auto i = 0; i < getProcessorCount(); ++i)
            {
                auto processor = getProcessor(i);
                mRootProcess->unregisterPostParticleProcess(processor->getCircularBuffer());
                processor->getGranulator().setActive(false);
            }
        }


        void GranulatorEffectInstance::onEnable()
        {
            for (auto i = 0; i < getProcessorCount(); ++i)
            {
                auto processor = getProcessor(i);
                mRootProcess->registerPostParticleProcess(processor->getCircularBuffer());
                processor->getGranulator().setActive(true);
            }
        }


        void GranulatorEffectInstance::onProcessorAdded(GranulatorEffectProcessor& processor)
        {
            // set all parameters of the new processor to the current parameter values
            
            processor.setDry(1.f - mDryWet->mValue);
            processor.setWet(mDryWet->mValue * audio::dbToA(mGain->mValue));
            mIsDirty.set();
            
            for (auto channel = 0; channel < processor.getGranulator().getChannelCount(); ++channel)
            {
                auto& c = processor.getGranulator().getChannel(channel);
                c.setDiffusion(mGranulatorDiffusion->mValue);
                c.setIrregularity(mGranulatorIrregularity->mValue);
                c.setAmplitude(mGranulatorAmplitude->mValue);
                c.setAmplitudeDev(mGranulatorAmplitudeDev->mValue);
                c.setTranspose(mGranulatorPitch->mValue);
                c.setTransposeDeviation(mGranulatorDetune->mValue);
                c.setAttackDecay(mGranulatorAttackDecay->mValue);
                c.setShape(audio::GranulatorInstance::Shape(mGranulatorShape->mValue));
            }
            
            SpatialEffectInstance::onProcessorAdded(processor);
            
        }



    }
    
}
