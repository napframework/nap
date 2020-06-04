#pragma once

// Spatial includes
#include <Spatial/Transformation/Transformation.h>


namespace nap {
    namespace spatial {
        
        class MixdownComponentInstance;
        
        /**
         * Spatial Dynamics.
         * This entire transformation might move to a Deform / Particlelevel transformation in the end.
         * Because it now affects the actual shape dimensions, so when lattice mode is on and spatial dynamics is on, it can cause rapid enabling/disabling of particles which could cause glitches.
         */
        class NAPAPI SpatialDynamicsTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            SpatialDynamicsTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            MixdownComponentInstance* mMixdownComponent = nullptr;
            ParameterFloat* mSpatialDynamicsDimensionsAmount = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(SpatialDynamicsTransformation, SpatialDynamicsTransformationInstance)
        
    }
}
