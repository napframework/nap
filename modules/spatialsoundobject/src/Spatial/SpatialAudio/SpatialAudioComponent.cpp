#include "SpatialAudioComponent.h"

// Spatial includes
#include <Spatial/SpatialAudio/MixdownComponent.h>
#include <Spatial/SpatialAudio/Source/SoundObjectSource.h>
#include <Spatial/SpatialAudio/Source/ExternalInputSource.h>
#include <Spatial/SpatialAudio/Source/TestSource.h>
#include <Spatial/SpatialAudio/MeasurementComponent.h>
#include <Spatial/Utility/AudioFunctions.h>
#include <Spatial/Core/SpatialFunctions.h>
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/RootProcess.h>
#include <Spatial/Transformation/SpatialTransformationComponent.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <nap/core.h>
#include <entity.h>

//// RTTI
RTTI_BEGIN_CLASS(nap::spatial::SpatialAudioComponent)
    RTTI_PROPERTY("InputEffects", &nap::spatial::SpatialAudioComponent::mInputEffects, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded);
    RTTI_PROPERTY("Effects", &nap::spatial::SpatialAudioComponent::mEffects, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded);
    RTTI_PROPERTY("PerceptionEffects", &nap::spatial::SpatialAudioComponent::mPerceptionEffects, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded);
    RTTI_PROPERTY("MaxParticleCount", &nap::spatial::SpatialAudioComponent::mMaxParticleCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialAudioComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("addExternalInput", &nap::spatial::SpatialAudioComponentInstance::addExternalInput)
    RTTI_FUNCTION("addTestSignal", &nap::spatial::SpatialAudioComponentInstance::addTestSignal)
    RTTI_FUNCTION("addSource", static_cast<bool(nap::spatial::SpatialAudioComponentInstance::*)(nap::spatial::SpatialSource*)>(&nap::spatial::SpatialAudioComponentInstance::addSource))
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {

        void SpatialAudioComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            // MeasurementComponent needs to initialise before particles are added by SpatialAudioComponent,
            // so MeasurementComponentInstance can connect to the particleAdded signal that results from particles being added in SpatialAudioComponentInstance
            // and so the EffectProcessors can connect to MeasurementComponentInstance's particleDataChanged signals.
            components.push_back(RTTI_OF(MeasurementComponent));
            components.push_back(RTTI_OF(SpatializationComponent));
            components.push_back(RTTI_OF(ParameterComponent));
            components.push_back(RTTI_OF(SpatialTransformationComponent));
            components.push_back(RTTI_OF(MixdownComponent));
        }


        bool SpatialAudioComponentInstance::init(utility::ErrorState& errorState)
        {
            // Get audioservice and components.
            mSpatialService = getEntityInstance()->getCore()->getService<SpatialService>();
            mAudioService = &mSpatialService->getAudioService();
            mSpatializationComponent = &getEntityInstance()->getComponent<SpatializationComponentInstance>();
            mParameterComponent = &getEntityInstance()->getComponent<ParameterComponentInstance>();
            mMeasurementComponent = &getEntityInstance()->getComponent<MeasurementComponentInstance>();
            mSpatialTransformationComponent = &getEntityInstance()->getComponent<SpatialTransformationComponentInstance>();
            std::vector<MixdownComponentInstance*> mixdownComponents;
            getEntityInstance()->getComponentsOfType(mixdownComponents);

            auto resource = getComponent<SpatialAudioComponent>();

            mParticleCount = resource->mMaxParticleCount;

            // Intialise parameters.
            initParameters();

            // Add soundobject input when input is connected at SpatializationComponent
            mSpatializationComponent->getInputConnectedSignal()->connect(mInputConnectedSlot);

            // Create particle gains controls for fading
            mParticleFader = std::make_unique<audio::ParallelNodeObjectInstance<audio::FastGainControlNode>>();
            mParticleFader->init(getParticleCount(), mAudioService->getNodeManager(), errorState);

            // create particles (default they are all inactive)
            for(int i = 0; i < getParticleCount(); i++)
            {
                auto particle = mSpatializationComponent->addParticle(mParticleFader->getOutputForChannel(i), glm::vec3(0, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec3(0, 0, 0), false);
                audio::FastGainControlNode* fader = mParticleFader->getChannel(i);
                auto particleController = std::make_unique<ParticleActivator>(*particle, *fader, mSpatializationComponent->getSpatialService().getRootProcess());
                mParticleActivators.emplace_back(std::move(particleController));
            }

            // initialise the "per input, per particle" effectchain
            mInputEffectChain.mEffects = resource->mInputEffects;
            mInputEffectChainInstance = mInputEffectChain.instantiate(getParticleCount(), *mAudioService, errorState, getEntityInstance());

            // creates mixer
            mMixer = std::make_unique<audio::ParallelNodeObjectInstance<audio::FastMixNode>>();
            mMixer->init(getParticleCount(), mAudioService->getNodeManager(), errorState);

            // initialise the "per particle" effectchain, and connect it to the mixer outputs
            mEffectChain.mEffects = resource->mEffects;
            mEffectChainInstance = mEffectChain.instantiate(getParticleCount(), *mAudioService, errorState, getEntityInstance());
            audio::IMultiChannelOutput* effectChainOutput = mEffectChainInstance->addProcessorChain(*mMixer, errorState);

            // initialise the "per particle" post-mixdown effectchain and connect it to the "per particle" effectchain
            mPerceptionEffectChain.mEffects = resource->mPerceptionEffects;
            mPerceptionEffectChainInstance = mPerceptionEffectChain.instantiate(getParticleCount(), *mAudioService, errorState, getEntityInstance());
            audio::IMultiChannelOutput* outputToParticleFaders = mPerceptionEffectChainInstance->addProcessorChain(*effectChainOutput, errorState);
            if (outputToParticleFaders == nullptr)
            {
                errorState.fail("Failed to initialize SpatialAudioComponent");
                return false;
            }

            // Connect the effect chain to the particle fader
            mParticleFader->IMultiChannelInput::connect(*outputToParticleFaders);

            // Pass the particles to the mixdown components
            for(int i = 0; i < getParticleCount(); i++)
            {
                for (auto& mixdownComponent : mixdownComponents)
                    // pass the output of the effect chain (pre perception) as the particle input for the mixdown
                    mixdownComponent->addParticle(*mSpatializationComponent->getParticle(i),
                                                  effectChainOutput->getOutputForChannel(i));
            }

            return true;
        }


        bool SpatialAudioComponentInstance::addSource(SpatialSource& resource, utility::ErrorState& errorState)
        {
            auto source = resource.instantiate(*mSpatialService, getParticleCount(), *mParameterComponent, errorState);
            if (source == nullptr)
            {
                errorState.fail("Failed to instantiate spatial source: ", errorState.toString().c_str());
                return false;
            }
            addSource(std::move(source), "enable");
            return true;
        }


        bool SpatialAudioComponentInstance::addSource(SpatialSource* resource)
        {
            utility::ErrorState errorState;
            if (!addSource(*resource, errorState))
            {
                Logger::warn("Failed to add spatial source: %s", errorState.toString().c_str());
                return false;
            }
            return true;
        }


        void SpatialAudioComponentInstance::addSource(std::unique_ptr<SpatialSourceInstance> source, const std::string& enableParameterName)
        {
            mSources.emplace_back(std::move(source));
            auto sourceIndex = mSources.size() - 1;
            SpatialSourceInstance* addedSource = mSources.back().get();

            // Add enable parameter
            const std::string namePrefix = addedSource->getNamePrefix();
            std::string prefixedName = namePrefix.empty() ? enableParameterName : namePrefix + "/" + enableParameterName;
            auto enableParam = &mParameterComponent->addParameterBool(prefixedName, false);
            enableParam->valueChanged.connect([&, sourceIndex, this](bool value){
                setSourceActive(sourceIndex, value);
            });
            mSourceEnableParameters.emplace_back(enableParam);

            // Notify MeasurementComponent that a new source has been created.
            mMeasurementComponent->sourceCreated(*addedSource);

            // Create the processing chain for this input.
            utility::ErrorState errorState;
            audio::IMultiChannelOutput* outputProcessor = mInputEffectChainInstance->addProcessorChain(*mSources.back(), errorState);
            assert(outputProcessor);
        }


        void SpatialAudioComponentInstance::addTestSignal()
        {
            auto source = std::make_unique<TestSource>(*mSpatialService, getParticleCount(), *mParameterComponent, "receives/testSignal");

            utility::ErrorState errorState;
            if (source->init(errorState))
                addSource(std::move(source), "enable");
            else
                Logger::warn("Failed to create test source: %s", errorState.toString().c_str());
        }



        void SpatialAudioComponentInstance::addSoundObjectSource(SpatializationComponentInstance& inputObject)
        {
            std::cout << "Adding sound object input." << std::endl;

            auto inputParameterComponent = inputObject.getEntityInstance()->findComponent<nap::ParameterComponentInstance>();

            auto mixdownComponent = inputObject.getEntityInstance()->findComponent<nap::spatial::MixdownComponentInstance>();
            auto source = std::make_unique<SoundObjectSource>(*mSpatialService, getParticleCount(), *mParameterComponent, "receives/" + inputParameterComponent->getGroupName(), inputObject.getEntityInstance());

            utility::ErrorState errorState;
            if (source->init(mixdownComponent, errorState))
                addSource(std::move(source), "enable");
            else
                Logger::warn("Failed to create sound object source: %s", errorState.toString().c_str());
        }


        void SpatialAudioComponentInstance::addExternalInput()
        {
            std::cout << "Adding external input." << std::endl;

            auto source = std::make_unique<ExternalInputSource>(mSpatializationComponent->getSpatialService(), getParticleCount(), *mParameterComponent, "");

            utility::ErrorState errorState;
            if (source->init(errorState))
                addSource(std::move(source), "externalInputEnable");
            else
                Logger::warn("Failed to create external input source: %s", errorState.toString().c_str());
        }


        float SpatialAudioComponentInstance::getInputLevelForSource(int index, int channel)
        {
            auto source = rtti_cast<DistributedSourceInstance>(mSources[index].get());
            assert(source->get_type().is_derived_from<DistributedSourceInstance>());
            return source->getInputChannelLevel(channel);
        }


        void SpatialAudioComponentInstance::update(double deltaTime)
        {
            updateEffectChains(deltaTime);
            updateParticles(deltaTime);
        }


        void SpatialAudioComponentInstance::updateEffectChains(double deltaTime)
        {
            mInputEffectChainInstance->update(deltaTime);
            mEffectChainInstance->update(deltaTime);
            mPerceptionEffectChainInstance->update(deltaTime);
        }


        void SpatialAudioComponentInstance::initParameters()
        {
            mDryGain = &mParameterComponent->addParameterFloat("directLoudness", 0.0, -48.0, 0.0);
            mDryGain->connect([&](float value){ mSpatializationComponent->setDryGain(audio::dbToA(value)); });
        }


        void SpatialAudioComponentInstance::updateParticles(double deltaTime)
        {

            // Get final particle positions from SpatialTransformComponent
            auto& particleTransforms = mSpatialTransformationComponent->getWorldSpaceParticleTransforms();

            // Update and activate/deactivate particles at SpatializationComponent/SpatialisationComponent.
            int activeParticleCount = std::min<int>(particleTransforms.size(), getParticleCount());
            for(int i = 0; i < activeParticleCount; i++)
            {
                mSpatializationComponent->getParticle(i)->setTransform(particleTransforms[i].mPosition, particleTransforms[i].mRotation, particleTransforms[i].mScale);
                mParticleActivators[i]->setActive(true);
            }
            for (int i = activeParticleCount; i < getParticleCount(); i++)
            {
                mParticleActivators[i]->setActive(false);
            }

            // Notify spatial sources that the number of active particles has changed
            if (activeParticleCount != mActiveParticleCount)
            {
                mActiveParticleCount = activeParticleCount;
                for (auto& source : mSources)
                    source->activeParticleCountChanged(mActiveParticleCount);
            }
        }


        void SpatialAudioComponentInstance::setSourceActive(unsigned int index, bool active)
        {
            auto inputProcessor = &mInputEffectChainInstance->getChainOutput(index);
            assert(inputProcessor != nullptr);

            // in the future when we have a NodeObject and a ParallelObject this can be done in 1 single call.
            if (active)
            {
                for(int i = 0; i < getParticleCount(); i++)
                    mMixer->getChannel(i)->inputs.enqueueConnect(*inputProcessor->getOutputForChannel(i));
            }
            else {
                for(int i = 0; i < getParticleCount(); i++)
                    mMixer->getChannel(i)->inputs.enqueueDisconnect(*inputProcessor->getOutputForChannel(i));
            }
        }


        SpatialAudioComponentInstance::ParticleActivator::ParticleActivator(Particle& particle, audio::FastGainControlNode& fader, RootProcess& rootProcess) : mParticle(particle), mFader(fader), mRootProcess(rootProcess)
        {
            mFader.getDestinationReachedSignal().connect(mDestinationReachedSlot);
        }


        void SpatialAudioComponentInstance::ParticleActivator::setActive(bool active)
        {
            if (active != mActive)
            {
                if (active)
                {
                    mParticle.setActive(true);
                    mFader.setGain(1.f, 250);
                    mRootProcess.registerParticle(mParticle.getOutputPin()->getNode());
                }
                else {
                    mFader.setGain(0.f, 250);
                }
                mActive = active;
            }
        }


        void SpatialAudioComponentInstance::ParticleActivator::destinationReached(audio::ControllerValue value)
        {
            if (value == 0)
            {
                mParticle.setActive(false);
                mRootProcess.unregisterParticle(mParticle.getOutputPin()->getNode());
            }
        }


    }
}
