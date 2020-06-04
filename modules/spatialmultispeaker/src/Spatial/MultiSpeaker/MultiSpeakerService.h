#pragma once

#include <nap/service.h>
#include <nap/signalslot.h>

namespace nap
{
    namespace spatial
    {
        
        class SpatialService;

        
        class NAPAPI MultiSpeakerServiceConfiguration : public ServiceConfiguration
        {
            RTTI_ENABLE(ServiceConfiguration)
        public:
            rtti::TypeInfo getServiceType() override;
            
            std::vector<int> mRouting = { }; ///< The routing of the multispeaker setup.
            std::string mSpeakerSetupFile = ""; ///< The path to the speaker setup file.
        };
    
        class NAPAPI MultiSpeakerService : public Service {
            RTTI_ENABLE(Service)
            
        public:
            MultiSpeakerService(ServiceConfiguration* configuration);
            
            bool init(nap::utility::ErrorState& errorState) override;
            
            /**
             * Signal triggered when the routing of the @MultiSpeakerSetup changes. The list of vacant channels are the output channel numbers that are not being used by the @MultiSpeakerSetup.
             */
            Signal<const std::vector<int>&> mVacantChannelsChangedSignal;

            /**
             * Updates the routing value in the configuration and updates the administration of vacant channels.
             * @param routing
             */
            void setMultiSpeakerRouting(const std::vector<int>& routing);
            
            /**
             * @return the spatial service.
             */
            SpatialService& getSpatialService() { return *mSpatialService; }
            
            /**
             * @return the multi speaker service configuration holding the speaker setup & routing.
             */
            MultiSpeakerServiceConfiguration& getMultiSpeakerServiceConfiguration() { return *getConfiguration<MultiSpeakerServiceConfiguration>(); }
            
            /**
             * @return a list of all channels that are not being used by the @MultiSpeakerSetup.
             */
            const std::vector<int>& getVacantChannels() const { return mVacantChannels; }
            
            
        protected:
            
            void registerObjectCreators(rtti::Factory& factory) override;
            
            void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;
            
        private:
            void updateVacantChannels(); // Creates the list of channels that are not used by the MultiSpeakerSetup.

            SpatialService* mSpatialService = nullptr;
            
            std::vector<int> mVacantChannels; // vacant channels that are not being used by the MultiSpeakerSetup.
            
        };
        
        
    }
    
}
