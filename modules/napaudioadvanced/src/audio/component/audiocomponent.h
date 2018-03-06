#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <rtti/objectptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
    
        class AudioComponentInstance;
        
        
        /**
         * Component that wraps an audio object that generates audio output for one or more channels.
         */
        class NAPAPI AudioComponent : public AudioComponentBase
        {
            RTTI_ENABLE(nap::audio::AudioComponentBase)
            DECLARE_COMPONENT(AudioComponent, AudioComponentInstance)
            
        public:
            AudioComponent() : AudioComponentBase() { }
            
            /**
             * The audio object that is wrapped by this component
             */
            rtti::ObjectPtr<AudioObject> mObject;
            
        private:
        };

        
        /**
         * Instance of a component that wraps an audio object that generates audio output for one or more channels
         */
        class NAPAPI AudioComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(nap::audio::AudioComponentBaseInstance)
            
        public:
            AudioComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            // Derived from AudioComponentBaseInstance
            int getChannelCount() const override { return mObject->getChannelCount(); }
            virtual OutputPin& getOutputForChannel(int channel) override { return mObject->getOutputForChannel(channel); }
            /**
             * Returns the wrapped audio object
             */
            AudioObjectInstance* getObject();
            
        private:
            std::unique_ptr<AudioObjectInstance> mObject = nullptr;
        };

    }
        
}
