#pragma once

#include <Spatial/Shape/Deform.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Implements a deformation based on a push-pull algorithm.
         */
        class NAPAPI PushPullDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)
            
        public:
            PushPullDeformInstance(DeformBase& deform) : DeformInstance(deform) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;
            
        private:
            float deformAmp(float distance, int type, float alpha, float beta);
            
            glm::vec3 twist(glm::vec3 pointR, float theta, glm::vec3 pointC, glm::vec3 dir);
            
            ParameterFloat* mAmount = nullptr;
            
            ParameterBool* mPush = nullptr;
            ParameterFloat* mTheta = nullptr;
            ParameterFloat* mPhi = nullptr;
            
            ParameterOptionList* mMode = nullptr;
            ParameterOptionList* mTension = nullptr;
            
            ParameterFloat* mIntensity = nullptr;
            ParameterFloat* mWidth = nullptr;
            
            ParameterOptionList* mDeformation = nullptr;
            ParameterFloat* mTwist = nullptr;
            ParameterVec3* mPosition = nullptr;
            
            ParameterBool* mEnable = nullptr;
        };
        
        DECLARE_DEFORM(PushPullDeform, PushPullDeformInstance)
        
    }
}
