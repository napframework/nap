#pragma once

// Spatial includes
#include <Spatial/Transformation/TransformationChainComponent.h>
#include <Spatial/Shape/ShapeComponent.h>
#include <Spatial/Core/SpatialFunctions.h>

// Nap includes
#include <component.h>

namespace nap
{
    namespace spatial
    {
        
        class SpatialTransformationComponentInstance;
        
        
        /**
         * The @SpatialTransformationComponent calculates and manages all relevant spatial transforms of the sound object. These are:
         * - the 'world space' sound object transform (the 'shell/cube' in which the particles are placed), calculated by the linked @TransformationChainComponent 'SoundObjectTransformationChain'.
         * - the 'shape space' particle positions, calculated by the linked @ConstellationComponent (dependend on the sound object dimensions).
         * - the 'world space' particle transforms, calculated by converting the shape space particle positions to world space and passing them one by one through the linked @TransformationChainComponent 'ParticleTransformationChain'.
         */
        class NAPAPI SpatialTransformationComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(SpatialTransformationComponent, SpatialTransformationComponentInstance)
            
        public:
            SpatialTransformationComponent() : Component() { }
            
            ComponentPtr<TransformationChainComponent> mSoundObjectTransformationChain; ///< ComponentPtr to the TransformationChain that determines the transform of the Sound Object.
            
            ComponentPtr<ShapeComponent> mShapeComponent; ///< ComponentPtr to the ShapeComponent that generates particle transforms.
            
            ComponentPtr<TransformationChainComponent> mParticleTransformationChain; ///< ComponentPtr to the TransformationChain that executes particle-level spatial dynamics / global transformations.

            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
        private:
            
        };

        
        class NAPAPI SpatialTransformationComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            SpatialTransformationComponentInstance(EntityInstance& entity, Component& resource);
            
            /**
             * Initialises the component.
             */
            bool init(utility::ErrorState& errorState) override;

            /**
             * Returns the sound object transform.
             */
            SpatialTransform& getSoundObjectTransform(){ return mSoundObjectTransform; }
            
            
            /**
             * Returns the 'world space' particle positions.
             */
            std::vector<SpatialTransform>& getWorldSpaceParticleTransforms(){ return mWorldSpaceParticleTransforms; }

            /**
             * Calculates all transforms.
             */
            virtual void update(double deltaTime) override;
            
        private:
            ComponentInstancePtr<TransformationChainComponent> mSoundObjectTransformationChain = { this, &SpatialTransformationComponent::mSoundObjectTransformationChain };
            ComponentInstancePtr<ShapeComponent> mShapeComponent = { this, &SpatialTransformationComponent::mShapeComponent };
            ComponentInstancePtr<TransformationChainComponent> mParticleTransformationChain = { this, &SpatialTransformationComponent::mParticleTransformationChain };
            
            // ___ Outputs ___
            SpatialTransform mSoundObjectTransform; ///< 'world space' sound object transform
            std::vector<SpatialTransform> mWorldSpaceParticleTransforms; ///< 'world space' particle transforms
            
            /// sound object position as parameter so other components and effects can access it through ParameterComponent
            /// (ported from EffectSoundComponent. This is maybe not the nicest method to pass this, should maybe be looked at after nap parameter update)
            Parameter* mPosition = nullptr;

        };
    }
    
}
