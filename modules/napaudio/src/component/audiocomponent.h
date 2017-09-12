#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <node/audionode.h>
#include <device/audiointerface.h>

namespace nap {
    
    namespace audio {
    
        class AudioComponentInstance;
        
        
        class NAPAPI AudioComponent : public Component {
            RTTI_ENABLE(nap::Component)
        public:
            AudioComponent() : nap::Component() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(AudioComponentInstance);
            }
            
            /**
             * Pointer to the audio interface the component runs on
             */
            ObjectPtr<AudioInterface> mAudioInterface;
            
        private:
        };

        
        class NAPAPI AudioComponentInstance : public ComponentInstance {
            RTTI_ENABLE(nap::ComponentInstance)
        public:
            AudioComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
            
            virtual OutputPin* getOutputForChannel(int channel) { return nullptr; }
            virtual int getChannelCount() const { return 0; }            
        };

    }
        
}
