//
//  SpatialDelayEffect.cpp
//  Project
//
//  Created by Casimir Geelhoed on 03/06/2019.
//
//

#include "SpatialDelayEffect.h"

// Spatial includes
#include <Spatial/Utility/AudioFunctions.h>
#include <Spatial/Core/ExposedDataComponent.h>

// Glm include
#include <glm/gtc/noise.hpp>


RTTI_DEFINE_CLASS(nap::spatial::SpatialDelayEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialDelayEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialDelayEffectInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        bool SpatialDelayEffectProcessor::initDSPChannel(int channel, audio::ChainInstance& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState)
        {
            auto delay = std::make_unique<audio::NodeObjectInstance<audio::FastDelayNode>>();
            delay->init(audioService.getNodeManager(), errorState);
            auto gain = std::make_unique<audio::NodeObjectInstance<audio::FastGainNode>>();
            gain->init(audioService.getNodeManager(), errorState);
            channelInstance.addObject(std::move(delay), errorState);
            channelInstance.addObject(std::move(gain), errorState);
            return true;
        }

        
        bool SpatialDelayEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            // we don't recalculate after parameter changes, because the delay times get calculated already in the update-loop (because of the Perlin noise). So we pass 'false' as last parameter.
            
            mFeedback = getParameterManager().addParameterFloat("feedback", 0.0, 0.0, 0.99);
            mFeedback->connect([&](float value){
                // set spatial delay feedback
                for (auto& delays : mCachedDelayNodes)
                    for (auto delay : delays)
                        delay->setFeedback(value);
            });
            mDistanceScale = getParameterManager().addParameterFloat("distanceScale", 0.0, 0.0, 30.0);
            mPeripheralScale = getParameterManager().addParameterFloat("peripheralScale", 0.0, 0.0, 30.0);
            mDopplerScale = getParameterManager().addParameterFloat("dopplerScale", 0.0, 0.0, 30.0);
            mRandomPatternScale = getParameterManager().addParameterFloat("randomPatternScale", 0.0, 0.0, 30.0);
            mDopplerUnisono = getParameterManager().addParameterBool("unisonEnable", false);
            mNoiseSpeed = getParameterManager().addParameterFloat("noiseModulationSpeed", 1.0, 0.0, 100.0);
            mNoiseDepth = getParameterManager().addParameterFloat("noiseModulationDepth", 0.0, 0.0, 1.0);
            mSmooth = getParameterManager().addParameterFloat("smooth", 0.05, 0.0, 1.0);
            
            // we connect the 'random value trigger' to recalculateRandomValues()
            mRandomValueTrigger = getParameterManager().addParameterBool("randomValueTrigger", false);
            mRandomValueTrigger->valueChanged.connect([&](float value){
                recalculateRandomValues();
                isTriggered = true;
            });
            
            
            auto& exposedData = entity->getComponent<ExposedDataComponentInstance>().getRoot();
            exposedData.expose<bool>("spatialDelayRandomValueTrigger", [&](){
                bool returnValue = isTriggered;
                isTriggered = false;
                return returnValue;
            });
            
            
            // dry wet parameter
            mDryWet = getParameterManager().addParameterFloat("dryWet", 0.0, 0.0, 1.0);
            mDryWet->connect([&](float value){ setDryWet(value); });

            // compensating gain
            mGain = getParameterManager().addParameterFloat("gain", 4.0, 0.0, 12.0);
            mGain->connect([&](float value){ setGain(audio::dbToA(value)); });
            
            // speed of sound in m/s
            mSpeedOfSound = getParameterManager().addParameterFloat("speedOfSound", 343.0, 100.0, 3000.0);
            
            recalculateRandomValues();
            
            return true;
        }
        
        void SpatialDelayEffectInstance::recalculateRandomValues()
        {
            
            // reset trigger parameter
            mRandomValueTrigger->mValue = false;
            
            // recalculate random values
            for(int i = 0; i < mDelayTimeRandomValues.size(); i++){
                for(int j = 0; j < mDelayTimeRandomValues[i].size(); j++){
                    mDelayTimeRandomValues[i][j] = math::random<float>(0., 1.);
                }
            }
            
        }
        
        void SpatialDelayEffectInstance::update(double deltaTime){
            
            if(isEnabled()){
                
                // increment accumulated time for simplex noise
                simplexTimePassed += deltaTime * mNoiseSpeed->mValue;
            
                // recalculate all particles for all processors
                recalculateAllParticles();

            }
            
        }
        
        
        void SpatialDelayEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
            
            auto particleMeasurer = getParticleMeasurer(particleIndex);
            
            // TODO distanceToSoundObjectCenter in measurementcomponent?
            float distanceToSoundObjectCenter = glm::distance(particleMeasurer->getPosition(), getSoundObjectTransform().mPosition);
            
            float delayCoefficient = 0.f;
            
            
            // input distance ('input')
            delayCoefficient += particleMeasurer->getDistanceToInput(processorIndex) * mDistanceScale->mValue;
            
            // center distance ('peripheral')
            delayCoefficient += distanceToSoundObjectCenter * mPeripheralScale->mValue;
            
            // vantage point distance ('doppler')
            if(mDopplerUnisono->mValue)
                delayCoefficient += glm::distance(getSoundObjectTransform().mPosition, particleMeasurer->getVantagePoint()) * mDopplerScale->mValue;
            else
                delayCoefficient += particleMeasurer->getDistanceToVantagePoint() * mDopplerScale->mValue;
            
            // static random delay ('random pattern')
            delayCoefficient += mDelayTimeRandomValues[processorIndex][particleIndex] * 10.0 * mRandomPatternScale->mValue;
            
            // calculate delay time in ms through the combined coefficient and the speed of sound.
            float delayTime = (delayCoefficient / mSpeedOfSound->mValue) * 1000.;
            
            // simplex noise multiplication
            float noiseValue = glm::simplex(glm::vec2(simplexTimePassed, mSimplexSeeds[processorIndex][particleIndex]));
            delayTime *= 1.0 + noiseValue * mNoiseDepth->mValue;
            
            // find delay
            auto delay = mCachedDelayNodes[processorIndex][particleIndex];
            
            // set spatial delay time
            delay->setTime(delayTime, mSmooth->mValue * 2000. + 1.);
        }
                                              
        void SpatialDelayEffectInstance::setGain(float gain)
        {
            for(int i = 0; i < getProcessorCount(); i++){
                for(int j = 0; j < getChannelCount(); j++){
                    mCachedGainNodes[i][j]->setGain(gain);
                }
            }
        }
        
        
        void SpatialDelayEffectInstance::onProcessorAdded(SpatialDelayEffectProcessor& processor)
        {
            
            std::vector<float> processorSimplexSeeds;
            std::vector<float> processorDelayTimeRandomValues;
            
            for(int i = 0; i < getChannelCount(); i++){
                processorSimplexSeeds.push_back(math::random<float>(0., 1000.0));
                processorDelayTimeRandomValues.push_back(math::random<float>(0., 1.));
            }
            
            mSimplexSeeds.push_back(processorSimplexSeeds);
            mDelayTimeRandomValues.push_back(processorDelayTimeRandomValues);
            
            // cache gain nodes
            std::vector<audio::SafePtr<audio::FastGainNode>> gainNodes;
            auto delayProcessor = rtti_cast<SpatialDelayEffectProcessor>(&processor);
            for(int i = 0; i < getChannelCount(); i++)
            {
                gainNodes.push_back(delayProcessor->getDSP(i)->getObject<audio::NodeObjectInstance<audio::FastGainNode>>(1)->get());
                
                // set the new gain to the value of the gain parameter
                gainNodes.back()->setGain(audio::dbToA(mGain->mValue));
            }
            mCachedGainNodes.push_back(gainNodes);
            
            // cache delay nodes
            std::vector<audio::SafePtr<audio::FastDelayNode>> delayNodes;
            for(int i = 0; i < getChannelCount(); i++)
            {
                delayNodes.push_back(delayProcessor->getDSP(i)->getObject<audio::NodeObjectInstance<audio::FastDelayNode>>(0)->get());
                
                // set the feedback of the new delay to the value of the feedback parameter
                delayNodes.back()->setFeedback(mFeedback->mValue);
            }
            mCachedDelayNodes.push_back(delayNodes);
            
            // set dry wet of the new processor to the value of the dry wet parameter
            processor.setDryWet(mDryWet->mValue);

            // trigger recalculate of this processor
            SpatialEffectInstance::onProcessorAdded(processor);
            
        }

        
    }
}


