#pragma once

// Nap includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/service/audioservice.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward delcarations
        class AudioObject;
        
        
        /**
         * Instance of a object that generates audio output for one or more channels
         */
        class NAPAPI AudioObjectInstance : public rtti::RTTIObject
        {
            RTTI_ENABLE()
            
        public:
            AudioObjectInstance(AudioObject& resource) : mResource(resource) { }
            
            /**
             * This method has to be overwritten by all descendants to initialize the instance.
             * Normally it will create all the Nodes owned by this instance and connect them.
             */
            virtual bool init(NodeManager& nodeManager, utility::ErrorState& errorState) = 0;
            
            /**
             * This method has to be overwritten to return the output pin corresponding to a given output channel of the object.
             */
            virtual OutputPin& getOutputForChannel(int channel) = 0;
            
            /**
             * This method has to be overwritten to return the number of channels of audio this object outputs.
             */
            virtual int getChannelCount() const = 0;
            
            /**
             * @return: the resource this instance is created from.
             */
            AudioObject& getResource() { return mResource; }
            
        private:
            AudioObject& mResource;
        };
        
        
        
        /**
         * AudioObject is a base class for objects that generate single- or multichannel audio using one or more Nodes in a DSP system.
         * AudioObjects can be linked together to build a more complex DSP system in a Graph.
         * AudioObject is a resource that can be instantiated.
         */
        class AudioObject : public rtti::RTTIObject
        {
            RTTI_ENABLE(rtti::RTTIObject)
            
        public:
            AudioObject() = default;
            
            /**
             * This method can be used during the initialization of a Graph of AudioObjects to acquire a pointer to the instance of this object in the graph.
             */
            AudioObjectInstance* getInstance() { return mInstance; }
            
            /**
             * This method spawns an instance of this resource.
             */
            std::unique_ptr<AudioObjectInstance> instantiate(NodeManager& nodeManager, utility::ErrorState& errorState);
            
        private:
            /**
             * This methods need to be overwritten by all descendants to return an instance of this resource.
             */
            virtual std::unique_ptr<AudioObjectInstance> createInstance() { return nullptr; }
            
            AudioObjectInstance* mInstance = nullptr;
        };
        
    
        
        /**
         * Base class for audio objects that contain a number of nodes of the same type, typically for performing multichannel processing.
         */
        class NAPAPI MultiChannelObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
            friend class MultiChannelObjectInstance;
            
        public:
            MultiChannelObject() = default;
            
            std::unique_ptr<AudioObjectInstance> createInstance() override;
            
        private:
            /**
             * This factory method has to be implemented by descendants to create a DSP Node for a certain channel.
             */
            virtual std::unique_ptr<Node> createNode(int channel, NodeManager& nodeManager) = 0;
            
            /**
             * This method has to be overwritten by descendants to return the number of nodes/channels that the instance of this object will own.
             */
            virtual int getChannelCount() const = 0;
        };

        
        
        /**
         * Instance of a MultiChannelObject. In most cases only the MultiChannelObject has to be overwritten to create your own MultiChannelObject type while this instance class can be left untouched.
         */
        class NAPAPI MultiChannelObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiChannelObjectInstance(MultiChannelObject& resource) : AudioObjectInstance(resource) { }
            
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
            OutputPin& getOutputForChannel(int channel) override { return *(*mNodes[channel]->getOutputs().begin()); }
            int getChannelCount() const override { return mNodes.size(); }
            
            /**
             * Returns the DSP node for the specified channel.
             */
            Node* getChannel(int channel);
            
        private:
            std::vector<std::unique_ptr<Node>> mNodes;
        };
                
    }
        
}


