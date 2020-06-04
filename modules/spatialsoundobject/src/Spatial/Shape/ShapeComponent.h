#pragma once

#include "Shape.h" // because we can't forward declare ResourcePtr<X> classes
#include "Deform.h" // because we can't forward declare ResourcePtr<X> classes

// Nap includes
#include <component.h>
#include <nap/resourceptr.h>

// glm::vec3
#include <glm/vec3.hpp>

namespace nap
{
    
    namespace spatial
    {
        // forward declarations.
        class ShapeInstance;
        class DeformInstance;
        class ShapeComponentInstance;
        class SpatialTransform;
        
        /**
         * Component that manages a list of shapes and a list of deforms. Each shape can output a vector with a @SpatialTransform for each active particle. One shape chosen from the list will be used as input for the list of deforms. Each deform modulates the transforms of each particle in time.
         */
        class NAPAPI ShapeComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(ShapeComponent, ShapeComponentInstance)
            
        public:
            ShapeComponent() : Component() { }
            ~ShapeComponent() { }
            
            std::vector<ResourcePtr<ShapeBase>> mShapes; ///< Property. List of shapes.
            
            std::vector<ResourcePtr<DeformBase>> mDeforms; ///< Property. Chain of deforms.
            
            virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        };


        /**
         * Instance of @ShapeComponent.
         */
        class NAPAPI ShapeComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
            
        public:
            ShapeComponentInstance(EntityInstance& entity, Component& resource);

            // Inherited from ComponentInstance
            bool init(utility::ErrorState& errorState) override;
            virtual void update(double deltaTime) override;

            /**
             * Calls the calls the calculateParticleTransforms()-function at the currently selected shape and applies the deforms to it.
             */
            std::vector<SpatialTransform> calculateParticleTransforms(const SpatialTransform& soundObjectTransform);

            /**
             * Returns vector containing the vertices of the soundobject for use of the visualization.
             */
            void getVerticesForVisualization(std::vector<glm::vec3>& out_vertices);
            
            /**
             * Returns vector containing the edges of the soundobject for use of the visualization. The edges are defined as pairs containing the indices of the two vertices that they are connecting.
             */
            bool getEdgesForVisualization(std::vector<std::pair<int,int>> & out_edges);
            
        private:
            std::vector<std::unique_ptr<ShapeInstance>> mShapes;
            std::vector<std::unique_ptr<DeformInstance>> mDeforms;

            ShapeInstance* mCurrentShape = nullptr;
            
            ParameterOptionList* mShapeType = nullptr;
            ParameterFloat* mSpeed = nullptr;
            double mTime = 0.;
        };
            
    }
    
}
