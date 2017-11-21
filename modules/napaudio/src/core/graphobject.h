#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <core/audioobject.h>
#include <core/graph.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Audio object that uses a graph to process it's output.
         */
        class NAPAPI GraphObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            GraphObject() : AudioObject() { }
            
            /**
             * Pointer to the graph resource that this object uses.
             */
            ObjectPtr<Graph> mGraph;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };

        
        /**
         * Instance of audio object that uses a graph to process it's output.
         */
        class NAPAPI GraphObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            GraphObjectInstance(GraphObject& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the component
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
            /**
             * Use this method to acquire an object within the graph by ID.
             * @return: a pointer to the object with the given ID.
             */
            AudioObjectInstance* getObject(const std::string& mID) { return mGraphInstance.getObject(mID); }
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            GraphInstance mGraphInstance;
        };
        
    }
    
}
