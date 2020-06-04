#pragma once

// Spatial includes
#include <Spatial/Transformation/Transformation.h>

// NAP includes
#include <nap/resource.h>


namespace nap
{
    
    namespace spatial
    {
    
        class TransformationChainComponentInstance;
        
        /**
         * An @ExternalTransformation is a transformation that applies an external @TransformationChainComponent's transformation as its transformation.
         * The external transformation chain can be set programmatically via 'setTransformationChain'.
         */
        class NAPAPI ExternalTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            ExternalTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) {}
            
            /** 
             * Does nothing.
             */
            void onInit(nap::EntityInstance* entity);
            
            /**
             * Creates 'enable' parameter. Has to be called directly after initialising. 
             * This is a separate function because we can't overload onInit with extra variables.
             */
            void createEnableParameter(bool defaultValue, bool shared);
            
            /**
             * Sets the external @TransformationChainComponent.
             */
            void setTransformationChain(TransformationChainComponentInstance* chain);
            
            /**
             * Applies the external @TransformationChainComponent' transformation if the 'enable' parameter is true.
             */
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation);
            
        private:
            TransformationChainComponentInstance* mExternalTransformationChainComponent = nullptr; ///< The external TransformationChainComponent.
            
            ParameterBool* mEnable = nullptr; ///< Enable parameter.
            
            
        };
        
        DECLARE_TRANSFORMATION(ExternalTransformation, ExternalTransformationInstance)
        
        
        /**
         * @SwitchExternalTransformation is like an @ExternalTransformation, but it allows to register multiple TransformationChains and creates an enum parameter to switch between them.
         */
        class SwitchExternalTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            SwitchExternalTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
                        
            void onInit(nap::EntityInstance* entity) { }
            
            /**
             * Sets the external transformation chains and creates an enum parameter.
             */
            void setTransformationChains(std::vector<TransformationChainComponentInstance*> chains, std::vector<std::string> names);
            
            /**
             * Applies the selected @TransformationChainComponent to the given position, scale and rotation.
             */
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation);
            
        private:
            std::vector<TransformationChainComponentInstance*> mExternalTransformationChainComponents; ///< The external TransformationChainComponents.

            ParameterOptionList* mSwitch = nullptr; ///< Enum parameter.
        };
        
        DECLARE_TRANSFORMATION(SwitchExternalTransformation, SwitchExternalTransformationInstance)

        
    }
    
}
    
