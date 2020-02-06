#include "advancedaudioservice.h"

// Std includes
#include <iostream>

// Nap includes
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/object/oscillator.h>
#include <audio/resource/audiofileio.h>
#include <audio/resource/equalpowertable.h>

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


        AdvancedAudioService::~AdvancedAudioService()
        {
        }


        void AdvancedAudioService::registerObjectCreators(rtti::Factory& factory)
        {
            auto audioService = getCore().getService<AudioService>();
            assert(audioService);
            factory.addObjectCreator(std::make_unique<WaveTableResourceObjectCreator>(audioService->getNodeManager()));
            factory.addObjectCreator(std::make_unique<AudioFileIOObjectCreator>(audioService->getNodeManager()));
            factory.addObjectCreator(std::make_unique<EqualPowerTableObjectCreator>(audioService->getNodeManager()));
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
