#pragma once

// Spatial includes
#include <Spatial/Core/ParameterManager.h>
#include <Spatial/Utility/ParameterTypes.h>

// NAP includes
#include <nap/resource.h>

// GLM includes
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


// Macro to define a Transformation named Type from instance InstanceType.
#define DECLARE_TRANSFORMATION(Type, InstanceType)  class NAPAPI Type : public Transformation<InstanceType> { RTTI_ENABLE(TransformationBase) };

namespace nap
{
    // forward declare
    class EntityInstance;

    namespace spatial
    {
     
        // forward declares
        class TransformationInstance;
        
        /**
         * Base class so we can rtti-define templated @Transformation resources. Literal copy of @EffectBase.
         */
        class NAPAPI TransformationBase : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            TransformationBase() = default;
            
            std::string mName; ///< property: 'Name' Used to set parameter prefixes.
            
            virtual std::unique_ptr<TransformationInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity, ParameterVec3& anchorPoint, const std::string& transformationChainName) { return nullptr; }
            
        };
                
        
        /**
         * An @Transformation is a resource that can instantiate @TransformationInstance instances.
         * (basically a copy of the 'Effect' class)
         */
        template <typename InstanceType>
        class NAPAPI Transformation : public TransformationBase
        {
            RTTI_ENABLE(TransformationBase)
            
        public:
            Transformation() = default;
            
            /**
             * Instantiates and initialises an EffectInstance. Returns nullptr if initialisation failed.
             */
            std::unique_ptr<TransformationInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity, ParameterVec3& anchorPoint, const std::string& transformationChainName) override
            {
                auto instance = std::make_unique<InstanceType>(*this);
                bool initSucceeded = instance->init(errorState, entity, anchorPoint, transformationChainName);
                if(initSucceeded)
                    return std::move(instance);
                    else
                        return nullptr;
                
            }
        };
        
        
        /**
         * A @TransformationInstance is an instance of a @Transformation.
         * It has an "apply" function that applies a transformation to a given transform, and a virtual update function.
         */
        class NAPAPI TransformationInstance
        {
            
            RTTI_ENABLE()
        public:
            
            TransformationInstance(TransformationBase& transformation) : mTransformation(transformation), mName(transformation.mName) { }

            virtual ~TransformationInstance() = default;
            
            /** 
             * Applies the transformation.
             */
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) = 0;
            
            
            /**
             * Initialises the TransformationInstance. Finds the @ParameterComponent and calls onInit().
             */
            bool init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity, ParameterVec3& anchorPoint, const std::string& transformationChainName);
            
            /**
             * Virtual update function that can be implemented. Called by TransformationChainComponentInstance.
             * This function will only be getting called if the owner of the TransformationInstance is calling the TransformationInstance::update() function in its update-loop.
             */
            virtual void update(double deltaTime) { }
            
            const std::string& getName() const { return mName; }
            
            
        protected:
            /**
             * Virtual function called during initialisation. Can be used to set up parameters etc.
             */
            virtual void onInit(nap::EntityInstance* entity) { };
            
            /**
             * Returns the 'anchorpoint' of this entity. The AnchorPoint parameter is managed by the first initialised TransformationChainComponent.
             */
            const glm::vec3& getAnchorPoint();
            
            /**
             * Returns a pointer to the resource.
             */
            template <typename T>
            T* getTransformation(){ return rtti_cast<T>(&mTransformation); };
            
            
            ParameterManager& getParameterManager() { return mParameterManager; };
            
        private:

            ParameterManager mParameterManager;

            TransformationBase& mTransformation;
            std::string mName;
            ParameterVec3* mAnchorPoint = nullptr;
            
        };
        
    }
    
}
