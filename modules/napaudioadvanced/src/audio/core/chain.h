#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/multiobject.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * A chain of AudioObjects whose inputs and outputs are connected serially.
         * A chain can never be empty.
         */
        class NAPAPI Chain : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            Chain() : AudioObject() { }
            
            std::vector<ResourcePtr<AudioObject>> mObjects;
            ResourcePtr<AudioObject> mInput = nullptr;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };
        
        
        class NAPAPI ChainInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            ChainInstance() : AudioObjectInstance() { }
            ChainInstance(const std::string& name) : AudioObjectInstance(name) { }

            OutputPin* getOutputForChannel(int channel) override { return mObjects.back()->getOutputForChannel(channel); }
            int getChannelCount() const override { return mObjects.back()->getChannelCount(); }
            void connect(unsigned int channel, OutputPin& pin) override { mObjects[0]->connect(channel, pin); }
            int getInputChannelCount() const override { return mObjects[0]->getInputChannelCount(); }
            
            /**
             * Use this method to acquire an object within the chain by index.
             * @return: a pointer to the object with the given index.
             */
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(mObjects[index].get()); }
            template <typename T>
            T* front() { return rtti_cast<T>(mObjects[0].get()); }
            template <typename T>
            T* back() { return rtti_cast<T>(mObjects.back().get()); }
            
            template <typename T>
            const T* getObject(unsigned int index) const { return rtti_cast<T>(mObjects[index].get()); }
            template <typename T>
            const T* front() const { return rtti_cast<T>(mObjects[0].get()); }
            template <typename T>
            const T* back() const { return rtti_cast<T>(mObjects.back().get()); }
            
            AudioObjectInstance* getObjectNonTyped(unsigned int index);
            
            bool addObject(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState);
            bool addObject(std::unique_ptr<AudioObjectInstance> object, utility::ErrorState& errorState);

            /**
             * Returns the number of object
             */
            int getObjectCount() const { return mObjects.size(); }

        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            
        private:
            /**
             * Overwrite this method to manipulate instantiation of objects in the chain.
             */
            virtual std::unique_ptr<AudioObjectInstance> instantiateObjectInChain(AudioObject& resource, NodeManager& audioService, utility::ErrorState& errorState);
            
            /**
             * Overwrite this method to manipulate the way two objects in the chain are connected.
             */
            virtual bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState);
        };
        
        
        
    }
    
}
