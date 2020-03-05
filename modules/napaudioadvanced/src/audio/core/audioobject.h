#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/core/multichannel.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward delcarations
        class AudioObject;
        
        
        /**
         * Instance of a object that generates audio output for one or more channels
         */
        class NAPAPI AudioObjectInstance : public IMultiChannelInput, public IMultiChannelOutput
        {
            RTTI_ENABLE()
            friend class AudioObject;
            
        public:
            AudioObjectInstance() = default;
            AudioObjectInstance(const std::string& name) : mName(name) { }

            AudioObjectInstance(const AudioObjectInstance&) = delete;
            AudioObjectInstance& operator=(const AudioObjectInstance&) = delete;

            /**
             * If multichannel input is implemented for this object it returns its input interface, otherwise nullptr.
             */
            IMultiChannelInput* getInput() { return dynamic_cast<IMultiChannelInput*>(this); }
            
            /**
             * If this object is instantiated from a resource this returns the mID of the resource.
             * Otherwise it returns an empty string.
             */
            const std::string& getName() const { return mName; }
            
        private:
            std::string mName = ""; // This is the mID of the resource that spawned the object. If the object has not been spawned by a resource this string remains empty.
        };
        
        
        
        /**
         * AudioObject is a base class for objects that generate single- or multichannel audio using one or more Nodes in a DSP system.
         * AudioObjects can be linked together to build a more complex DSP system in a Graph.
         * AudioObject is a resource that can be instantiated.
         */
        class NAPAPI AudioObject : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            AudioObject() = default;
            
            /**
             * This method can be used during the initialization of a Graph of AudioObjects to acquire a pointer to the instance of this object in the graph.
             */
            AudioObjectInstance* getInstance() { return mInstance; }
            
            /**
             * This method spawns an instance of this resource.
             */
            template <typename T>
            std::unique_ptr<T> instantiate(NodeManager& nodeManager, utility::ErrorState& errorState);
            
        private:
            /**
             * This methods need to be overwritten by all descendants to return an instance of this resource.
             */
            virtual std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) = 0;
            
            AudioObjectInstance* mInstance = nullptr;
        };
        
        
        template <typename T>
        std::unique_ptr<T> AudioObject::instantiate(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = createInstance(nodeManager, errorState);
            if (instance == nullptr)
                return nullptr;
            instance->mName = mID;
            mInstance = instance.release();
            return std::unique_ptr<T>(rtti_cast<T>(mInstance));
        }
        
        

    }
        
}


