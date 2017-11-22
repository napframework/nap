#pragma once

// Nap includes
#include "component.h"
#include "componentptr.h"
#include <nap/objectptr.h>

// Audio includes
#include <core/audionode.h>
#include <core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
    
        class AudioComponentInstance;
        
        
        /**
         * Component that wraps an audio object that generates audio output for one or more channels.
         */
        class NAPAPI AudioComponent : public Component
        {
            RTTI_ENABLE(nap::Component)
            DECLARE_COMPONENT(AudioComponent, AudioComponentInstance)
            
        public:
            AudioComponent() : nap::Component() { }
            
            /**
             * The audio object that is wrapped by this component
             */
            ObjectPtr<AudioObject> mObject;
            
        private:
        };

        
        /**
         * Instance of a component that wraps an audio object that generates audio output for one or more channels
         */
        class NAPAPI AudioComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(nap::ComponentInstance)
            
        public:
            AudioComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            /**
             * Returns the wrapped audio object
             */
            AudioObjectInstance* getObject();
            
        protected:
            /**
             * Returns the node system's manager that the audio runs on
             */
            NodeManager& getNodeManager();
            
        private:
            std::unique_ptr<AudioObjectInstance> mObject = nullptr;
        };

    }
        
}
