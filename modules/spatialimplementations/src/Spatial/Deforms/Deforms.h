#pragma once

#include <Spatial/Shape/Deform.h>

namespace nap
{

    namespace spatial
    {

        static const int DEFORM_MAX_PARTICLES = 512;


        class NAPAPI VibrateDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)

        public:
            VibrateDeformInstance(DeformBase& deform) : DeformInstance(deform) { }

            virtual void onInit(nap::EntityInstance* entity) override;

            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;

            void update(double deltaTime) override;

        private:
            ParameterBool* mEnable = nullptr;

            ParameterFloat* mAmount = nullptr;
            ParameterFloat* mReach = nullptr;
            ParameterFloat* mSpeed = nullptr;

            float mXSpeeds [DEFORM_MAX_PARTICLES];
            float mYSpeeds [DEFORM_MAX_PARTICLES];
            float mZSpeeds [DEFORM_MAX_PARTICLES];

            glm::vec3 mOffsets [DEFORM_MAX_PARTICLES] = {glm::vec3(0,0,0)};

            double mAccumulatedTime = 0.;

        };

        DECLARE_DEFORM(VibrateDeform, VibrateDeformInstance)


        class NAPAPI WaveDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)

        public:
            WaveDeformInstance(DeformBase& deform) : DeformInstance(deform) { }

            virtual void onInit(nap::EntityInstance* entity) override;

            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;

            void update(double deltaTime) override;

        private:
            ParameterBool* mEnable = nullptr;

            ParameterFloat* mAmount = nullptr;
            ParameterFloat* mReach = nullptr;
            ParameterFloat* mSpeed = nullptr;
            
            ParameterFloat* mWaveLength = nullptr;
            ParameterFloat* mAngle = nullptr;
            
            ParameterBool* mWorldSpace = nullptr;

            double mAccumulatedTime = 0.;

        };

        DECLARE_DEFORM(WaveDeform, WaveDeformInstance)


        class NAPAPI BuzzDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)

        public:
            BuzzDeformInstance(DeformBase& deform) : DeformInstance(deform) { }

            virtual void onInit(nap::EntityInstance* entity) override;

            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;

            void update(double deltaTime) override;

        private:
            ParameterBool* mEnable = nullptr;

            ParameterFloat* mAmount = nullptr;
            ParameterFloat* mReach = nullptr;
            ParameterFloat* mSpeed = nullptr;
            
            double mAccumulatedTime = 0.;

        };

        DECLARE_DEFORM(BuzzDeform, BuzzDeformInstance)


        class NAPAPI ShakeDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)

        public:
            ShakeDeformInstance(DeformBase& deform) : DeformInstance(deform) { }

            virtual void onInit(nap::EntityInstance* entity) override;

            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;

            void update(double deltaTime) override;

        private:
            ParameterBool* mEnable = nullptr;

            ParameterFloat* mAmount = nullptr;
            ParameterFloat* mReach = nullptr;
            ParameterFloat* mDuration = nullptr;
            ParameterBool* mTrigger = nullptr;

            void onTrigger(bool);
            Slot<bool> triggerSlot = { this, &ShakeDeformInstance::onTrigger };

            void regenerateTargetOffsets();

            bool mMoving = false;
            float mFractionIncrement = 0.;
            float mFraction = 1.;

            glm::vec3 mOffsets [DEFORM_MAX_PARTICLES] = {glm::vec3(0,0,0)};
            glm::vec3 mPreviousOffsets [DEFORM_MAX_PARTICLES] = {glm::vec3(0,0,0)};
            glm::vec3 mTargetOffsets [DEFORM_MAX_PARTICLES] = {glm::vec3(0,0,0)};

        };

        DECLARE_DEFORM(ShakeDeform, ShakeDeformInstance)

    }

}
