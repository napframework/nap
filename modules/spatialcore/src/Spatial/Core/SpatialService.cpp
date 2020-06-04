#include "SpatialService.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Stereo/StereoSpeakerSetup.h>
#include <Spatial/Masking/MaskingComponent.h>
#include <Spatial/Masking/MaskingOccluderComponent.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <nap/core.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialService)
    RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        SpatialService::SpatialService(ServiceConfiguration* configuration) : Service(configuration)
        {
        }
        
        
        bool SpatialService::init(nap::utility::ErrorState& errorState)
        {
            mAudioService = getCore().getService<audio::AudioService>();
            assert(mAudioService);
            auto& nodeManager = mAudioService->getNodeManager();
            mRootProcess = nodeManager.makeSafe<RootProcess>(*mAudioService, 50);

            initLevelMeters();
            nodeManager.mChannelCountChangedSignal.connect(mChannelCountChangedSlot);
            
            return true;
        }


        void SpatialService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<StereoSpeakerSetupObjectCreator>(*this));
        }
        
        
        void SpatialService::update(double deltaTime)
        {
        	// update masking occluder transforms
			
        	for (MaskingOccluderComponentInstance * maskingOccluderComponent : mMaskingOccluderComponents)
        	{
				maskingOccluderComponent->updateTransform();
			}
			
            // update masking values
			
            for (MaskingComponentInstance * maskingComponent : mMaskingComponents)
            {
            	maskingComponent->updateMasking();
			}

			
			// update panners
			
        	for (auto& panner : mSpeakerSetups)
        		panner->update();
        }
        

        void SpatialService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
        {
            dependencies.emplace_back(RTTI_OF(audio::AudioService));
        }
        

        void SpatialService::shutdown()
        {
        }


        audio::OutputPin& SpatialService::getInput(int channel)
        {
            return mInputNodes[channel]->audioOutput;
        }


        float SpatialService::getInputLevel(int channel)
        {
            return mInputLevelMeters[channel]->getLevel();
        }


        int SpatialService::getInputChannelCount() const
        {
            return mAudioService->getNodeManager().getInputChannelCount();
        }
		
		
        void SpatialService::registerSpeakerSetup(SpeakerSetup& panner)
        {
            mSpeakerSetups.emplace(&panner);
            mSpeakerSetupRegisteredSignal(&panner);
        }
        
        
        void SpatialService::unregisterSpeakerSetup(SpeakerSetup& panner)
        {
            panner.setActive(false);
            mSpeakerSetups.erase(&panner);
            mSpeakerSetupUnregisteredSignal(&panner);
        }
        
        
        void SpatialService::connectSpatializationComponent(SpatializationComponentInstance& component)
        {
            mSpatializationComponents.emplace(&component);
            for (auto& panner : mSpeakerSetups)
                if (panner->getIsActive())
                    panner->connect(component);
            
            for (auto& particle : component.getParticles())
            {
                if (particle->isActive())
                    mRootProcess->registerParticle(particle->getOutputPin()->getNode());
            }
			mSpatializationComponentRegisteredSignal(&component);
        }
        
        
        void SpatialService::disconnectSpatializationComponent(SpatializationComponentInstance& component)
        {
            mSpatializationComponents.erase(&component);
            for (auto& panner : mSpeakerSetups)
                if (panner->getIsActive())
                    panner->disconnect(component);
            
            for (auto& particle : component.getParticles())
            {
                if (particle->isActive())
                    mRootProcess->unregisterParticle(particle->getOutputPin()->getNode());
            }
			mSpatializationComponentUnregisteredSignal(&component);
        }
		
		
        void SpatialService::registerMaskingComponent(MaskingComponentInstance& component)
        {
            mMaskingComponents.emplace(&component);
        }


        void SpatialService::unregisterMaskingComponent(MaskingComponentInstance& component)
        {
            mMaskingComponents.erase(&component);
        }


        void SpatialService::registerMaskingOccluderComponent(MaskingOccluderComponentInstance& component)
        {
            mMaskingOccluderComponents.emplace(&component);
        }


        void SpatialService::unregisterMaskingOccluderComponent(MaskingOccluderComponentInstance& component)
        {
            mMaskingOccluderComponents.erase(&component);
        }

        
        void SpatialService::initLevelMeters()
        {
            for (auto& levelMeter : mInputLevelMeters)
                getRootProcess().unregisterInputLevelMeter(*levelMeter);
            mInputNodes.clear();
            mInputLevelMeters.clear();

            auto& nodeManager = mAudioService->getNodeManager();
            for (auto channel = 0; channel < nodeManager.getInputChannelCount(); ++channel)
            {
                auto input = nodeManager.makeSafe<audio::InputNode>(nodeManager);
                input->setInputChannel(channel);
                auto levelMeter = nodeManager.makeSafe<audio::LevelMeterNode>(nodeManager, 50.f, false);
                levelMeter->input.connect(input->audioOutput);
                levelMeter->setType(audio::LevelMeterNode::Type::PEAK);
                getRootProcess().registerInputLevelMeter(*levelMeter);
                mInputNodes.emplace_back(std::move(input));
                mInputLevelMeters.emplace_back(std::move(levelMeter));
            }

        }


        void SpatialService::channelCountChanged(audio::NodeManager& nodeManager)
        {
            initLevelMeters();
            mInputChannelCountChangedSignal(nodeManager.getInputChannelCount());
            mOutputChannelCountChangedSignal(nodeManager.getOutputChannelCount());
        }

    }
}
