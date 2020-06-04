#pragma once

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Transformation/Transformation.h> // necessary because of useage as ResourcePtr<X>

// Nap includes
#include <component.h>
#include <nap/resourceptr.h>


namespace nap
{
    // forward declare
    class EntityInstance;

    namespace spatial
    {
        
        class TransformationChainComponentInstance;
        
        
        /**
         * The @TransformationChainComponent is a component that holds a list of linearly chained @Transformations.
         * Also, additional @ExternalTransformations (such as follow transformations and global group transformations)
         * can be appended to this list programmatically by calling TransformationChainComponentInstance::addExternalTransformation(..).
         */
        class NAPAPI TransformationChainComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(TransformationChainComponent, TransformationChainComponentInstance)
            
        public:
            TransformationChainComponent() : Component() { }
            
            std::vector<ResourcePtr<TransformationBase>> mTransformations; ///< Embedded property. The transformations in the chain.
            
            std::string mName;
            
            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        };

        

        /**
         * Instance of @TransformationChainComponent.
         */
        class NAPAPI TransformationChainComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            TransformationChainComponentInstance(EntityInstance& entity, Component& resource);
            
            /**
             * Initialises the Component. 
             * Creates an 'anchorPoint' parameter if it doesn't exist already 
             * and instantiates the Transformation instances from the vector of Resources.
             */
            bool init(utility::ErrorState& errorState) override;
            
            /**
             * Applies all transformations to the given @SpatialTransform.
             */
            void apply(SpatialTransform& spatialTransform);
            
            /**
             * Applies all transformations to the given position, scale and rotation references.
             */
            void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation);
            
            /**
             * Applies all transformations up to (not including) the transformation with name excludeFrom, 
             * and all transformations after (not including) the transformation with name excludeTo.
             * Used for visualisation. For example, for the path visualisation, you would call applyExcluding(.., "path", "spatialDynamics"), to exclude every transformation from path until spatialDynamics.
             * 'excludeUntil' can be left empty to exclude everything from 'excludeFrom' and onwards.
             */
            void applyExcluding(SpatialTransform& spatialTransform, std::string excludeFrom, std::string excludeUntil);
            void applyExcluding(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation, std::string excludeFrom, std::string excludeUntil);


            /**
             * Applies all transformations except the 'excludedTransformation'.
             */
            void applyExcluding(SpatialTransform& spatialTransform, std::string excludedTransformation);
            void applyExcluding(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation, std::string excludedTransformation);

            
            /**
             * Adds an external transformation to the end of the transformation chain.
             * @param defaultEnable: whether the transformation is enabled by default
             * @param sharedEnable: whether the transformation's enable parameter is shared with transformations with the same name in other transformationchains
             */
            void addExternalTransformation(std::string name, TransformationChainComponentInstance* transformationChain, bool defaultEnable, bool sharedEnable);

            /**
             * Adds an external switch transformation to the end of the transformation chain.
             */
            void addSwitchExternalTransformation(std::string name, std::vector<TransformationChainComponentInstance*> transformationChains, std::vector<std::string> names);

            /**
             * Updates all @TransformationInstances.
             */
            virtual void update(double deltaTime) override;
            
            /**
             * Returns a transformation by type. (Used by PathVisualizationComponent to find the PathTransformation)
             */
            TransformationInstance* getTransformationByName(std::string name);
            
        private:
            std::vector<std::unique_ptr<TransformationInstance>> mTransformations; ///< The transformation instances.
            
            std::string mName;
            
            ParameterVec3* mAnchorPoint = nullptr; ///< The 'anchor point' parameter.
        };
        
    }
}
