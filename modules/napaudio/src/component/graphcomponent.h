#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <graph/audioobject.h>
#include <graph/audiograph.h>

namespace nap
{
    
    namespace audio
    {
    
        class GraphComponentInstance;
        
        
        class NAPAPI GraphObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            GraphObject() : AudioObject() { }
            
            ObjectPtr<Graph> mGraph;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };

        
        class NAPAPI GraphObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            GraphObjectInstance(GraphObject& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the component
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
            AudioObjectInstance* getObject(const std::string& mID) { return mGraphInstance.getObject(mID); }
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            GraphInstance mGraphInstance;
        };
        
    }
    
}
