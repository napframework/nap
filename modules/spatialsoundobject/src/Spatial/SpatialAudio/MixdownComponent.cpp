#include "MixdownComponent.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/ExposedDataComponent.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <mathutils.h>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::MixdownComponent)
    RTTI_PROPERTY("ChannelCount", &nap::spatial::MixdownComponent::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MixdownComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getMeasuredLevel", &nap::spatial::MixdownComponentInstance::getMeasuredLevel)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        
        void MixdownComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.emplace_back(RTTI_OF(ParameterComponent)); // for envelope follower parameters
            components.emplace_back(RTTI_OF(ExposedDataComponent)); // for envelope follower value
        }
        
        
        
        MixdownComponentInstance::MixdownComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)
        {
        }

        
        MixdownComponentInstance::~MixdownComponentInstance()
        {
            for (auto& bufferNode : mBufferNodes)
                mSpatialService->getRootProcess().unregisterMixdownBuffer(*bufferNode);
            if (mLevelMeterNode != nullptr)
                mSpatialService->getRootProcess().unregisterInputLevelMeter(*mLevelMeterNode);
        }

    
        bool MixdownComponentInstance::init(utility::ErrorState& errorState)
        {
            mResource = getComponent<MixdownComponent>();
            mAudioService = getEntityInstance()->getCore()->getService<audio::AudioService>();
            if (mAudioService == nullptr)
            {
                errorState.fail("MixdownComponent can't find AudioService");
                return false;
            }
            auto& nodeManager = mAudioService->getNodeManager();
            mSpatialService = getEntityInstance()->getCore()->getService<SpatialService>();
            if (mSpatialService == nullptr)
            {
                errorState.fail("MixdownComponent can't find SpatialService");
                return false;
            }

            std::set<audio::OutputPin*> pins;
            for (auto channel = 0; channel < mResource->mChannelCount; ++channel)
            {
                auto mixNode = nodeManager.makeSafe<audio::FastMixNode>(nodeManager);
                auto gainNode = nodeManager.makeSafe<audio::FastGainNode>(nodeManager);
                    
                auto bufferNode = nodeManager.makeSafe<audio::BufferNode>(nodeManager);
                pins.emplace(&bufferNode->audioOutput);
                gainNode->audioInput.connect(mixNode->audioOutput);

                
                // Add envelope follower for spatial dynamics to first channel (if we will have multichannel mixdowns, it should average all channels instead of only measure the first channel?
                // For the time being an irrelevant question, because we only use mono mixdown.
                if (channel == 0)
                {
                    mEnvelopeFollowerNode = nodeManager.makeSafe<audio::EnvelopeFollowerNode>(nodeManager);
                    mEnvelopeFollowerNode->audioInput.connect(gainNode->audioOutput);
                    bufferNode->audioInput.connect(mEnvelopeFollowerNode->audioOutput);
                    mLevelMeterNode = nodeManager.makeSafe<audio::LevelMeterNode>(nodeManager, 50.f, false);
                    mLevelMeterNode->input.connect(bufferNode->audioOutput);
                    mSpatialService->getRootProcess().registerInputLevelMeter(*mLevelMeterNode);
                }
                else {
                    bufferNode->audioInput.connect(gainNode->audioOutput);
                }
                
                mSpatialService->getRootProcess().registerMixdownBuffer(*bufferNode);
                mMixNodes.emplace_back(std::move(mixNode));
                mGainNodes.emplace_back(std::move(gainNode));
                mBufferNodes.emplace_back(std::move(bufferNode));
            }

            // Create the mixdown spatial outputs
            for (auto pin : pins)
            {
                auto output = std::make_unique<SpatialOutput>(pin, glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec3(0, 0, 0));
                mMixdown.emplace_back(std::move(output));
            }

            mParticles.resize(mResource->mChannelCount);

            if (mMixdown.size() != mResource->mChannelCount)
            {
                errorState.fail("Unable to create mixdown.");
                return false;
            }
            
            // add envelope follower parameters
            
            auto& parameterComponent = getEntityInstance()->getComponent<ParameterComponentInstance>();

            mEnvelopeFollowerAttack = &parameterComponent.addParameterFloat("envelopeFollower/attack", 10.0, 1., 1000.);
            
            mEnvelopeFollowerRelease = &parameterComponent.addParameterFloat("envelopeFollower/release", 250., 1., 1000.);

            mEnvelopeFollowerAttack->connect([&](float x){ setEnvelopeFollowerAttack(x); });
            mEnvelopeFollowerRelease->connect([&](float x){ setEnvelopeFollowerRelease(x); });
            
            
            // expose envelopefollower data
            auto& exposedData = getEntityInstance()->getComponent<ExposedDataComponentInstance>().getRoot();
            exposedData.expose<float>("envelopeFollower/value", [&](){ return getMeasuredLevel(); });

            
            return true;
        }
        
        
        void MixdownComponentInstance::update(double deltaTime)
        {
            if (mDirty)
            {
                updateTransforms();
                mDirty = false;
            }
        }


        float MixdownComponentInstance::getMeasuredLevel() const
        {
            return mLevelMeterNode->getLevel();
        }


        float MixdownComponentInstance::getEnvelopeFollowerOutput() const
        {
            return mEnvelopeFollowerNode->getValue();
        }


        SpatialOutput* MixdownComponentInstance::getOutput(int channel)
        {
            return mMixdown[channel].get();
        }
        

        int MixdownComponentInstance::getChannelCount() const
        {
            return mMixdown.size();
        }
        

        void MixdownComponentInstance::setEnvelopeFollowerAttack(float attack)
        {
            mEnvelopeFollowerNode->setAttack(attack);
        };
        

        void MixdownComponentInstance::setEnvelopeFollowerRelease(float release)
        {
            mEnvelopeFollowerNode->setRelease(release);
        };


        void MixdownComponentInstance::addParticle(Particle& particle, audio::OutputPin* mixdownInput)
        {
            if (mixdownInput == nullptr)
                mixdownInput = particle.getOutputPin();
            auto channel = getParticleCount() % mResource->mChannelCount;
            mParticles[channel].emplace(std::make_unique<ParticleInput>(&particle, mixdownInput));
            if (particle.isActive())
                mMixNodes[channel]->inputs.enqueueConnect(*mixdownInput);
            particle.getTransformChangedSignal()->connect(mParticleTransformChangedSlot);
            particle.getActiveChangedSignal()->connect(mParticleActiveChangedSlot);
            adjustGains();
            mDirty = true;
        }

        
        void MixdownComponentInstance::adjustGains()
        {
            for (auto channel = 0; channel < mResource->mChannelCount; ++channel)
            {
                int channelParticleCount = 0;
                for (auto& particleInput : mParticles[channel])
                    if (particleInput->mParticle->isActive())
                        channelParticleCount++;
                mGainNodes[channel]->setGain(audio::ControllerValue(mResource->mChannelCount));
            }
        }
        
        
        void MixdownComponentInstance::updateTransforms()
        {
            for (auto channel = 0; channel < mMixdown.size(); ++channel)
            {
                glm::vec3 meanPosition = glm::vec3(0, 0, 0);
                glm::vec3 meanScale = glm::vec3(0, 0, 0);
                
                int activeParticleCount = 0;
                
                for (auto& particleInput : mParticles[channel])
                {
                    if (particleInput->mParticle->isActive())
                    {
                        activeParticleCount++;
                        meanPosition = meanPosition + particleInput->mParticle->getPosition();
                        meanScale = meanScale + particleInput->mParticle->getScale();
                    }
                }
                
                if(activeParticleCount > 0){
                    meanPosition = meanPosition / float(activeParticleCount);
                    meanScale = meanScale / float(activeParticleCount);
                }
                
                mMixdown[channel]->setTransform(meanPosition, mMixdown[channel]->getRotation(), meanScale);
            }
        }
        
        
        int MixdownComponentInstance::getChannelForParticle(const Particle& particle) const
        {
            for (auto channel = 0; channel < mParticles.size(); ++channel)
            {
                auto& particleInput = mParticles[channel];
                auto particlePtr = &particle;
                if (std::find_if(particleInput.begin(), particleInput.end(), [particlePtr](auto& element){ return element->mParticle == particlePtr; }) != particleInput.end())
                    return channel;
            }
            return -1;
        }


        int MixdownComponentInstance::getParticleCount() const
        {
            int result = 0;
            for (auto& particleInputs : mParticles)
                result += particleInputs.size();
            return result;
        }

        
        void MixdownComponentInstance::particleActiveChanged(Particle& particle)
        {
            auto channel = getChannelForParticle(particle);
            assert(channel >= 0);
            if (particle.isActive())
                mMixNodes[channel]->inputs.enqueueConnect(*particle.getOutputPin());
            else
                mMixNodes[channel]->inputs.enqueueDisconnect(*particle.getOutputPin());
            adjustGains();
            mDirty = true;
        }

        
    }
        
}
