#pragma once

// Implementations includes
#include <Spatial/Transformations/ModulationTransformation.h> // for modulations

// Spatial includes
#include <Spatial/Shape/Deform.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Port from provided python script. Implements a deformation based on an attraction-repulsion algorithm.
         */
        class NAPAPI AttractDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)
            
        public:
            AttractDeformInstance(DeformBase& deform) : DeformInstance(deform) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;
            
            virtual void update(double deltaTime) override;
            
        private:
            enum AttractionType { Linear, Gaussian, Hyperbolic };
            
            float mAccumulatedTime = 0.;
            std::vector<Modulation> mModulations;
            
            float deformAmp(float distance, AttractionType type, float scope);
            
            ParameterBool* mEnable = nullptr;
            
            ParameterOptionList* mType = nullptr;
            ParameterFloat* mCurve = nullptr;
            ParameterFloat* mAmount = nullptr;
            ParameterVec3* mOffset = nullptr;
            ParameterFloat* mAzimuth = nullptr;
            ParameterFloat* mElevation = nullptr;
            ParameterFloat* mTwist = nullptr;
                        
        };
        
        DECLARE_DEFORM(AttractDeform, AttractDeformInstance)
    }
}
