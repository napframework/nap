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

            std::vector<AudioObjectPtr> mObjects;
            AudioObjectPtr mOutput;
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
        
    }
    
}
