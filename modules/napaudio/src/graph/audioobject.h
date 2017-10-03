#pragma once

// Nap includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

// Audio includes
#include <node/audionode.h>
#include <node/audionodemanager.h>
#include <service/audioservice.h>
#include <graph/audiograph.h>

namespace nap {
    
    namespace audio {
        
        // Forward declares
        class AudioObjectInstance;
        class MultiChannelObjectInstance;
        
        
        class AudioObject : public rtti::RTTIObject {
            RTTI_ENABLE(rtti::RTTIObject)
            
        public:
            AudioObject() = default;
            
            AudioObjectInstance* getInstance() { return mInstance; }
            std::unique_ptr<AudioObjectInstance> instantiate(NodeManager& nodeManager, utility::ErrorState& errorState);
            
        private:
            virtual std::unique_ptr<AudioObjectInstance> createInstance() { return nullptr; }
            
            AudioObjectInstance* mInstance = nullptr;
        };
        
        
        /**
         * Instance of a object that generates audio output for one or more channels
         */
        class NAPAPI AudioObjectInstance {
            RTTI_ENABLE()
            
        public:
            AudioObjectInstance(AudioObject& resource) : mResource(resource) { }
            
            virtual bool init(NodeManager& nodeManager, utility::ErrorState& errorState) { return true; }
            
            virtual OutputPin& getOutputForChannel(int channel) = 0;
            virtual int getChannelCount() const = 0;
            
            AudioObject& getResource() { return mResource; }
            
        private:
            AudioObject& mResource;
        };
        
    
        /**
         * Component that generates audio output for one or more channels
         */
        class NAPAPI MultiChannelObject : public AudioObject {
            RTTI_ENABLE(AudioObject)
            
            friend class MultiChannelObjectInstance;
            
        public:
            MultiChannelObject() = default;
            
            std::unique_ptr<AudioObjectInstance> createInstance() override;
            
        private:
            virtual std::unique_ptr<Node> createNode(int channel, NodeManager& nodeManager) = 0;
            virtual int getChannelCount() const = 0;
        };

        
        /**
         * Instance of a object that generates audio output for one or more channels
         */
        class NAPAPI MultiChannelObjectInstance : public AudioObjectInstance {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiChannelObjectInstance(MultiChannelObject& resource) : AudioObjectInstance(resource) { }
            
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
            OutputPin& getOutputForChannel(int channel) override { return *(*mNodes[channel]->getOutputs().begin()); }
            int getChannelCount() const override { return mNodes.size(); }
            
            Node* getChannel(int channel);
            
        private:
            std::vector<std::unique_ptr<Node>> mNodes;
        };
                
    }
        
}


