#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/graph.h>

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
            ResourcePtr<Graph> mGraph = nullptr;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };
        
        
        /**
         * Instance of audio object that uses a graph to process it's output.
         */
        class NAPAPI GraphObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            GraphObjectInstance() : AudioObjectInstance() { }
            GraphObjectInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Initialize the object
            bool init(Graph& graph, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Use this method to acquire an object within the graph by ID.
             * @return: a pointer to the object with the given ID.
             */
            template <typename T>
            T* getObject(const std::string& mID) { return mGraphInstance.getObject<T>(mID); }
            
            /**
             * Non typed version of @getObject().
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& mID) { return mGraphInstance.getObjectNonTyped(mID); }
            
            /**
             * Adds an object to the graph. The graph takes over ownership.
             */
            AudioObjectInstance& addObject(std::unique_ptr<AudioObjectInstance> object) { return mGraphInstance.addObject(std::move(object)); }
            
            /**
             * Adds an object to the graph and marks it to be the graph's input. The graph takes over ownership of the object.
             */
            AudioObjectInstance& addInput(std::unique_ptr<AudioObjectInstance> object)  { return mGraphInstance.addInput(std::move(object)); }
            
            /**
             * Adds an object to the graph and marks it to be the graph's output. The graph takes over ownership of the object.
             */
            AudioObjectInstance& addOutput(std::unique_ptr<AudioObjectInstance> object) { return mGraphInstance.addOutput(std::move(object)); }


            // Multichannel implementation
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            void connect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override;
            
        private:
            GraphInstance mGraphInstance;
        };                

    }

}
