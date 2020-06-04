#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/MixdownComponent.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainNode.h>

// Audio includes
#include <audio/node/outputnode.h>

// Nap includes
#include <component.h>
#include <componentptr.h>



namespace nap
{
    
    namespace spatial
    {
    
        class MixdownComponent;
        class MixdownOutputComponentInstance;
        class Parameter;
        class RootProcess;
        class MultiSpeakerService;
        
        /**
         * Component that outputs the signal of a mixdown component to a channel of the DAC.
         * Note: for now, this only works with mono mixdowns.
         */
        class NAPAPI MixdownOutputComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(MixdownOutputComponent, MixdownOutputComponentInstance)
            
        public:
            MixdownOutputComponent() : Component() { }
            
            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
            ComponentPtr<MixdownComponent> mMixdownComponent; ///< Property. The mixdown to output.
            
        private:
        };

        
        class NAPAPI MixdownOutputComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            MixdownOutputComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
        private:
            ParameterBool* mEnable = nullptr; ///< Parameter. Whether the mixdown output is enabled or not.
            ParameterInt* mOutputChannel = nullptr; ///< Parameter. The output channel.
            ParameterFloat* mVolume = nullptr; ///< Parameter. The volume of the output.
            
            Slot<const std::vector<int>&> mVacantChannelsChangedSlot = { this, &MixdownOutputComponentInstance::vacantChannelsChanged };
            void vacantChannelsChanged(const std::vector<int>&) { updateChannelAndEnable(); }
            
            /**
             * Sets the output channel if it is valid, and registers/deregisters the outputnode based on the enable parameter and the vacant channels of the spatial service.
             * Called after a change of the enable parameter or a change of the output channel count in spatial service.
             */
            void updateChannelAndEnable();
            
            void setGain(float gain);
            
            ComponentInstancePtr<MixdownComponent> mMixdownComponent = { this, &MixdownOutputComponent::mMixdownComponent }; ///< The mixdown to output.

            audio::SafeOwner<audio::FastGainNode> mGainNode;
            audio::SafeOwner<audio::OutputNode> mOutputNode;
                        
            RootProcess* mRootProcess = nullptr;
            
            MultiSpeakerService* mMultiSpeakerService = nullptr;
            
            bool mEnabled = false; /// < Whether the mixdown output is enabled (determined by the the enable parameter and the valid channel variable).
            
            bool mValidChannel = false; /// < True if the selected channel is a valid vacant channel.
            
        };
        
    }
    
}
