#pragma once

// Std includes
#include <vector>

// Nap includes
#include <nap/objectptr.h>

// Nap includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

// Audio includes
#include <node/audionode.h>
#include <node/audionodemanager.h>

namespace nap {
    
    namespace audio {
        
        // Forward declarations
        class GraphInstance;
        class AudioObject;
        class AudioObjectInstance;
        
        
        using AudioObjectPtr = ObjectPtr<AudioObject>;
        
        
        class NAPAPI Graph : public rtti::RTTIObject {
            RTTI_ENABLE(rtti::RTTIObject)
        public:
            Graph() = default;
            Graph(NodeManager& nodeManager) : mNodeManager(&nodeManager)  { }

            std::vector<AudioObjectPtr> mObjects;
            AudioObjectPtr mOutput;
            
            NodeManager& getNodeManager() { return *mNodeManager; }
            
        private:
            NodeManager* mNodeManager = nullptr;
        };
        
        
        class NAPAPI GraphInstance {
        public:
            GraphInstance() = default;
            bool init(Graph& resource, utility::ErrorState& errorState);
            
            AudioObjectInstance& getOutput() { return *mOutput; }
            const AudioObjectInstance& getOutput() const { return *mOutput; }

        private:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            AudioObjectInstance* mOutput = nullptr;
        };
        
        
        using GraphObjectCreator = rtti::ObjectCreator<Graph, NodeManager>;
        
    }
    
}
