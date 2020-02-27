#pragma once

// Std includes
#include <vector>

// Nap includes
#include <nap/resourceptr.h>
#include <rtti/object.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audioobject.h>


namespace nap
{
    
    namespace audio
    {
        
        using AudioObjectPtr = ResourcePtr<AudioObject>;
        
        
        /**
         * The Graph manages a number of different audio objects that are connected together to represent a DSP network to perform a specific task of mono or multichannel audio processing.
         * Internally it resolves links between the objects and manages order of initialization accordingly.
         */
        class NAPAPI Graph : public Resource
        {
            RTTI_ENABLE(Resource)
        public:
            Graph() = default;

            /**
             * The audio objects managed by the graph that are part of it's DSP network.
             */
            std::vector<AudioObjectPtr> mObjects;
            
            /**
             * Pointer to an audio object in the graph that will be polled for output in order to present audio output.
             */
            ResourcePtr<AudioObject> mOutput = nullptr;
            
            /**
             * Pointer to an effect object in the graph where audio input will be connected to.
             */
            ResourcePtr<AudioObject> mInput = nullptr;
        };
        
        
        /**
         * Instance of Graph that manages a number of different audio objects, connected together to represent a DSP network to perform a specific task of mono or multichannel audio processing.
         */
        class NAPAPI GraphInstance
        {
            RTTI_ENABLE()
            
        public:
            GraphInstance() = default;
            virtual ~GraphInstance() = default;

            GraphInstance(const GraphInstance&) = delete;
            GraphInstance& operator=(const GraphInstance&) = delete;

            bool init(Graph& resource, audio::NodeManager& nodeManager, utility::ErrorState& errorState);

            
            /**
             * @return: an object within this graph by ID.
             */
            template <typename T>
            T* getObject(const std::string& name)
            {
                return rtti_cast<T>(getObjectNonTyped(name));
            }
            
            /**
             * Non typed version of getObject() for use in python.
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& name);
            
            /**
             * Returns the output object of the graph as specified in the resource
             */
            AudioObjectInstance* getOutput() { return mOutput; }
            const AudioObjectInstance* getOutput() const { return mOutput; }

            /**
             * Returns the input object of the graph as specified in the resource
             */
            AudioObjectInstance* getInput() { return mInput; }
            const AudioObjectInstance* getInput() const { return mOutput; }
            
            /**
             * Adds an object to the graph. The graph takes over ownership.
             */
            AudioObjectInstance& addObject(std::unique_ptr<AudioObjectInstance> object);
            
            /**
             * Adds an object to the graph and marks it to be the graph's input. The graph takes over ownership of the object.
             */
            AudioObjectInstance& addInput(std::unique_ptr<AudioObjectInstance> object);
            
            /**
             * Adds an object to the graph and marks it to be the graph's output. The graph takes over ownership of the object.
             */
            AudioObjectInstance& addOutput(std::unique_ptr<AudioObjectInstance> object);

        protected:
            /**
             * @return: all objects within the graph.
             */
            const std::vector<std::unique_ptr<AudioObjectInstance>>& getObjects() const { return mObjects; }
            
            /**
             * @return: the audio service that this graph runs on.
             */
            NodeManager& getNodeManager() { return *mNodeManager; }

        private:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            AudioObjectInstance* mOutput = nullptr;
            AudioObjectInstance* mInput = nullptr;
            NodeManager* mNodeManager = nullptr;
        };
        
        
    }
    
}
