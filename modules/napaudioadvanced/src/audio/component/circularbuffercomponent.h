#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/node/circularbuffernode.h>

namespace nap
{
    
    namespace audio
    {
    
        class CircularBufferComponentInstance;
        
        
        class NAPAPI CircularBufferComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(CircularBufferComponent, CircularBufferComponentInstance)
            
        public:
            CircularBufferComponent() : Component() { }
            
        public:
            // Properties
            nap::ComponentPtr<AudioComponentBase> mInput; ///<  property: 'Input' The component whose audio output to rout to the circular buffer.
            
            std::vector<int> mChannelRouting = { 0 }; ///< property: 'Routing' The size of this vector indicates the number of channels in the circular buffer.
            ///< Each element in the array represents one channel of the circular buffer.
            ///< The value of the element indicates the channel from the input that will be routed to the corresponding channel.

            int mBufferSize = 65536;
            
        private:
        };

        
        class NAPAPI CircularBufferComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            CircularBufferComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            CircularBufferNode* getChannel(unsigned int channel);
            
        private:
            std::vector<SafeOwner<CircularBufferNode>> mNodes; // Circular buffer for each channel
            nap::ComponentInstancePtr<AudioComponentBase> mInput = { this, &CircularBufferComponent::mInput }; // Pointer to the component whose output will routed to the circular buffer.
        };
        
    }
        
}
