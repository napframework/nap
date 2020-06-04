#pragma once

// Spatial include
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/ParameterManager.h>

// NAP includes
#include <nap/resource.h>
#include <entity.h>


// Macro to define a Deform named Type from instance InstanceType.
#define DECLARE_DEFORM(Type, InstanceType)  class NAPAPI Type : public Deform<InstanceType> { RTTI_ENABLE(DeformBase) };


namespace nap
{

    namespace spatial
    {
        
        class DeformInstance;
        
        /**
         * Base class so we can rtti-define templated @Transformation resources.
         */
        class NAPAPI DeformBase : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            DeformBase() = default;
            
            std::string mName; ///< property: 'Name' Used to set parameter prefixes.
            
            virtual std::unique_ptr<DeformInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity) { return nullptr; }
            
        };
        
        
        /**
         * A @Deform is a resource that can instantiate @DeformInstance instances.
         * (basically a copy of the 'Effect' class)
         */
        template <typename InstanceType>
        class NAPAPI Deform : public DeformBase
        {
            RTTI_ENABLE(DeformBase)
            
        public:
            Deform() = default;
            
            /**
             * Instantiates and initialises an EffectInstance. Returns nullptr if initialisation failed.
             */
            std::unique_ptr<DeformInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity) override
            {
                auto instance = std::make_unique<InstanceType>(*this);
                bool initSucceeded = instance->init(errorState, entity);
                if(initSucceeded)
                    return std::move(instance);
                    return nullptr;
                
            }
        };
        
        
        /**
         * Instance of @Deform.
         */
        class NAPAPI DeformInstance
        {
            
            RTTI_ENABLE()
        public:
            
            DeformInstance(DeformBase& deform) : mDeform(deform) { }

            virtual ~DeformInstance() = default;
            
            virtual bool init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity);
            
            /**
             * Applies the transformation.
             */
            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) = 0;
            
            virtual void update(double deltaTime) { }
            
            const std::string& getName() const { return mDeform.mName; }
            
            
        protected:
            /**
             * Virtual function called during initialisation. Can be used to set up parameters etc.
             */
            virtual void onInit(nap::EntityInstance* entity) { };
            
            /**
             * Returns a pointer to the resource.
             */
            template <typename T>
            T* getDeform() { return rtti_cast<T>(&mDeform); };
            
            ParameterManager& getParameterManager() { return mParameterManager; };
            
        private:
            ParameterManager mParameterManager;
            
            DeformBase& mDeform;
            
        };
        
    }
    
}
