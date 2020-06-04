#include "MultiSpeakerService.h"

// Local includes
#include "MultiSpeakerSetup.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/service/audioservice.h>

// NAP includes
#include <nap/core.h>


RTTI_BEGIN_CLASS(nap::spatial::MultiSpeakerServiceConfiguration)
    RTTI_PROPERTY("Routing", &nap::spatial::MultiSpeakerServiceConfiguration::mRouting, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("SpeakerSetupFile", &nap::spatial::MultiSpeakerServiceConfiguration::mSpeakerSetupFile, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MultiSpeakerService)
    RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {
        
        
        rtti::TypeInfo MultiSpeakerServiceConfiguration::getServiceType()
        {
            return RTTI_OF(MultiSpeakerService);
        }

        
        MultiSpeakerService::MultiSpeakerService(ServiceConfiguration* configuration) : Service(configuration)
        {
        }
        
        
        bool MultiSpeakerService::init(nap::utility::ErrorState& errorState)
        {
            mSpatialService = getCore().getService<SpatialService>();
            assert(mSpatialService);

            // update vacant channels when the output channel count changed
            mSpatialService->mOutputChannelCountChangedSignal.connect([&](int x) { updateVacantChannels(); });
            
            updateVacantChannels();
            
            return true;
        }
        
        
        void MultiSpeakerService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<MultiSpeakerSetupObjectCreator>(*this));
        }
        
        void MultiSpeakerService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
        {
            dependencies.push_back(RTTI_OF(SpatialService));
        }

        
        void MultiSpeakerService::setMultiSpeakerRouting(const std::vector<int>& routing)
        {
            getMultiSpeakerServiceConfiguration().mRouting = routing;
            updateVacantChannels();
        }
        
        
        void MultiSpeakerService::updateVacantChannels()
        {
            mVacantChannels.clear();

            // create list of vacant channels
            auto& nodeManager = mSpatialService->getAudioService().getNodeManager();
            int dacChannelCount = nodeManager.getOutputChannelCount();
            std::vector<int> speakerChannels = getMultiSpeakerServiceConfiguration().mRouting;
            for(int i = 0; i < dacChannelCount; i++)
            {
                if(std::find(speakerChannels.begin(), speakerChannels.end(), i) == speakerChannels.end())
                    mVacantChannels.push_back(i);
            }

            mVacantChannelsChangedSignal(mVacantChannels);
            
        }
        
        

    }
}
