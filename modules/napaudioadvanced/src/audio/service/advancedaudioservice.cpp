// Std includes
#include <iostream>

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "advancedaudioservice.h"

#include <audio/service/audioservice.h>
#include <audio/core/graph.h>
#include <audio/core/voice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AdvancedAudioService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
		AdvancedAudioService::AdvancedAudioService(ServiceConfiguration* configuration) :
			Service(configuration)
		{
		}
        
        void AdvancedAudioService::registerObjectCreators(rtti::Factory& factory)
        {
            auto nodeManager = &getCore().getService<AudioService>(rtti::ETypeCheck::IS_DERIVED_FROM)->getNodeManager();
            assert(nodeManager);
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(*nodeManager));
            factory.addObjectCreator(std::make_unique<VoiceObjectCreator>(*nodeManager));
        }
        
        
        void AdvancedAudioService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
        {
            dependencies.emplace_back(RTTI_OF(AudioService));
        }

        
        bool AdvancedAudioService::init(nap::utility::ErrorState& errorState)
        {
            return true;
        }
        

    }
}
