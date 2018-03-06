#pragma once

// Nap includes
#include <nap/service.h>

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Service registering a few Object Creators specifically for advanced audio
         */
        class NAPAPI AdvancedAudioService final : public Service
        {
            RTTI_ENABLE(nap::Service)
            
        public:
            AdvancedAudioService() = default;
            
            /**
             * Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

            bool init(nap::utility::ErrorState& errorState) override;
        
        };
        
        
    }
}
