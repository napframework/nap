#include "StereoSpeakerSetup.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/SpatialTypes.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <mathutils.h>
#include <nap/logger.h>
#include <entity.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::StereoSpeakerSetup)
    RTTI_PROPERTY("OutputChannels", &nap::spatial::StereoSpeakerSetup::mOutputChannels, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Region", &nap::spatial::StereoSpeakerSetup::mRegion, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        StereoSpeakerSetup::ParticleProcessor::ParticleProcessor(StereoSpeakerSetup& speakerSetup, SpatializationComponentInstance& soundObject, Particle& particle) : mParticle(&particle), mSpeakerSetup(&speakerSetup)
        {
            auto& nodeManager = mSpeakerSetup->getSpatialService().getAudioService().getNodeManager();
			
            mGainNode = nodeManager.makeSafe<audio::GainNode>(nodeManager);
            mGainNode->inputs.connect(*particle.getOutputPin());
            mGain = soundObject.getDryGain();
			updateGain();
			
            mStereoPannerNode = nodeManager.makeSafe<audio::StereoPannerNode>(nodeManager);
            mStereoPannerNode->leftInput.enqueueConnect(mGainNode->audioOutput);
            mStereoPannerNode->rightInput.enqueueConnect(mGainNode->audioOutput);
			
            pan(mParticle->getPosition(), mParticle->getRotation(), mParticle->getScale());
            if (particle.isActive())
                mSpeakerSetup->connectParticleProcessor(*this);
			
            particle.getTransformChangedSignal()->connect(transformChangedSlot);
            particle.getOutputPinChangedSignal()->connect(outputPinChangedSlot);
            particle.getActiveChangedSignal()->connect(activeChangedSlot);
            soundObject.getDryGainChangedSignal()->connect(gainChangedSlot);
            speakerSetup.getMasterVolumeChangedSignal().connect(masterVolumeChangedSlot);
        }
        
        
        audio::OutputPin& StereoSpeakerSetup::ParticleProcessor::getLeftOutput()
        {
            return mStereoPannerNode->leftOutput;
        }
        
        
        audio::OutputPin& StereoSpeakerSetup::ParticleProcessor::getRightOutput()
        {
            return mStereoPannerNode->rightOutput;
        }

        
        void StereoSpeakerSetup::ParticleProcessor::pan(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale)
        {
            auto x = position.x;
            auto region = mSpeakerSetup->mRegion;
            auto panning = math::fit(x, - region * 0.5f, region * 0.5f, 0.f, 1.f);
            mStereoPannerNode->setPanning(panning);
        }
		
		
        void StereoSpeakerSetup::ParticleProcessor::updateGain()
        {
			mGainNode->setGain(mGain * mSpeakerSetup->getMasterVolume(), 1);
		}
        
        
        void StereoSpeakerSetup::ParticleProcessor::transformChanged(const SpatialOutput& spatialOutput)
        {
            pan(spatialOutput.getPosition(), spatialOutput.getRotation(), spatialOutput.getScale());
        }
        
        
        void StereoSpeakerSetup::ParticleProcessor::outputPinChanged(const SpatialOutput& spatialOutput)
        {
            mStereoPannerNode->leftInput.enqueueConnect(*spatialOutput.getOutputPin());
            mStereoPannerNode->rightInput.enqueueConnect(*spatialOutput.getOutputPin());
        }
        
        
        void StereoSpeakerSetup::ParticleProcessor::activeChanged(const Particle& particle)
        {
            if (particle.isActive())
                mSpeakerSetup->connectParticleProcessor(*this);
            else
                mSpeakerSetup->disconnectParticleProcessor(*this);
        }
        
        
        void StereoSpeakerSetup::ParticleProcessor::gainChanged(audio::ControllerValue gain)
        {
        	mGain = gain;
			
        	updateGain();
        }
		
		
        void StereoSpeakerSetup::ParticleProcessor::masterVolumeChanged(audio::ControllerValue gain)
        {
			updateGain();
		}


		StereoSpeakerSetup::~StereoSpeakerSetup()
		{
		}
        
        bool StereoSpeakerSetup::init(utility::ErrorState& errorState)
        {
            auto& nodeManager = getAudioService().getNodeManager();
            auto& process = *getProcess<audio::ParentProcess>();
            
            for (auto channel = 0; channel < 2; ++channel)
            {
                mMixers.emplace_back(nodeManager.makeSafe<audio::FastMixNode>(nodeManager));
                mOutputs.emplace_back(nodeManager.makeSafe<audio::OutputNode>(nodeManager, false));
                mOutputs.back()->setOutputChannel(mOutputChannels[channel]);
                mOutputs.back()->audioInput.enqueueConnect(mMixers.back()->audioOutput);
                process.addChild(*mOutputs.back());
            }
            
            return SpeakerSetup::init(errorState);
        }

 
        void StereoSpeakerSetup::particleAdded(SpatializationComponentInstance& component, Particle& particle)
        {
            mParticleProcessors.emplace_back(std::make_unique<ParticleProcessor>(*this, component, particle));
        }
        
        
        void StereoSpeakerSetup::particleRemoved(SpatializationComponentInstance&, Particle& particle)
        {
            auto particlePtr = &particle;
            auto it = std::find_if(mParticleProcessors.begin(), mParticleProcessors.end(), [particlePtr](auto& panner){ return panner->getParticle() == particlePtr; });
            if (it != mParticleProcessors.end())
                mParticleProcessors.erase(it);
        }
		
		
        void StereoSpeakerSetup::connectParticleProcessor(ParticleProcessor& particleProcessor)
        {
            mMixers[0]->inputs.enqueueConnect(particleProcessor.getLeftOutput());
            mMixers[1]->inputs.enqueueConnect(particleProcessor.getRightOutput());
        }
        
        
        void StereoSpeakerSetup::disconnectParticleProcessor(ParticleProcessor& particleProcessor)
        {
            mMixers[0]->inputs.enqueueDisconnect(particleProcessor.getLeftOutput());
            mMixers[1]->inputs.enqueueDisconnect(particleProcessor.getRightOutput());
        }

    
    }
    
}

