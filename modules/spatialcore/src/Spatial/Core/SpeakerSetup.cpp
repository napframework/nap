#include "SpeakerSetup.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/RootProcess.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/core/process.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpeakerSetup)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
    
        SpeakerSetup::SpeakerSetup(SpatialService& service) : Resource(), mService(service)
        {
        }
        
        
        SpeakerSetup::~SpeakerSetup()
        {
            mService.unregisterSpeakerSetup(*this);
        }
        
        
        bool SpeakerSetup::init(utility::ErrorState& errorState)
        {
            mService.registerSpeakerSetup(*this);
            return true;
        }
        
        
        void SpeakerSetup::connect(SpatializationComponentInstance& component)
        {
            for (auto& particle : component.getParticles())
                particleAdded(component, *particle);
            component.getParticleCreatedSignal()->connect(particleCreatedSlot);
        }
        
        
        void SpeakerSetup::disconnect(SpatializationComponentInstance& component)
        {
            for (auto& particle : component.getParticles())
                particleRemoved(component, *particle);
            component.getParticleCreatedSignal()->disconnect(particleCreatedSlot);
        }
        
        
        audio::AudioService& SpeakerSetup::getAudioService()
        {
            return mService.getAudioService();
        }

        
        void SpeakerSetup::particleCreated(SpatializationComponentInstance& component, Particle& particle)
        {
            particleAdded(component, particle);
        }
        
        
        audio::SafeOwner<audio::Process> SpeakerSetup::createProcess()
        {
            auto& nodeManager = getAudioService().getNodeManager();
            return nodeManager.makeSafe<audio::ParentProcess>(nodeManager, getSpatialService().getRootProcess().getThreadPool(), getSpatialService().getRootProcess().getAsyncObserver());
        }


        void SpeakerSetup::setActive(bool active)
        {
            if (active == mIsActive)
                return;

            mIsActive = active;
            if (mIsActive)
            {
                for (auto& spatializationComponent : getSpatialService().getSpatializationComponents())
                    connect(*spatializationComponent);
                getSpatialService().getRootProcess().registerSpeakerSetup(*getProcess<audio::Process>());
            }
            else {
                getSpatialService().getRootProcess().unregisterSpeakerSetup(*getProcess<audio::Process>());
                for (auto& spatializationComponent : getSpatialService().getSpatializationComponents())
                    disconnect(*spatializationComponent);
                onDisconnect();
            }
        }




    }

}
