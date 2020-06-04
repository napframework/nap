#include "Deforms.h"

// Spatial includes
#include <Spatial/Core/SpatialFunctions.h>
#include <Spatial/Core/ParameterComponent.h>

// Nap includes
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::VibrateDeformInstance)
RTTI_END_CLASS
RTTI_DEFINE_CLASS(nap::spatial::VibrateDeform)


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::WaveDeformInstance)
RTTI_END_CLASS
RTTI_DEFINE_CLASS(nap::spatial::WaveDeform)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::BuzzDeformInstance)
RTTI_END_CLASS
RTTI_DEFINE_CLASS(nap::spatial::BuzzDeform)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ShakeDeformInstance)
RTTI_END_CLASS
RTTI_DEFINE_CLASS(nap::spatial::ShakeDeform)

namespace nap
{

    namespace spatial
    {

        void VibrateDeformInstance::onInit(nap::EntityInstance* entity)
        {
            // generate seeds
            for(int i = 0; i < DEFORM_MAX_PARTICLES; i++){
                mXSpeeds[i] = math::random<float>(-1., 1.);
                mYSpeeds[i] = math::random<float>(-1., 1.);
                mZSpeeds[i] = math::random<float>(-1., 1.);
            }

            mAmount = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);
            mReach = getParameterManager().addParameterFloat("radius", 1.0, 0.0, 10.0);
            mSpeed = getParameterManager().addParameterFloat("speed", 1.0, 0.0, 10.0);
            
            mEnable = getParameterManager().addParameterBool("enable", false);

        }


        void VibrateDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            if(mEnable->mValue)
            {
                for(int i = 0; i < transforms.size() && i < DEFORM_MAX_PARTICLES; i++)
                    transforms[i].mPosition += mOffsets[i] * mReach->mValue * mAmount->mValue;
            }
        }


        void VibrateDeformInstance::update(double deltaTime)
        {
            mAccumulatedTime += deltaTime * mSpeed->mValue;

            for(int i = 0; i < DEFORM_MAX_PARTICLES; i++){
                mOffsets[i].x = sinf(mAccumulatedTime * mXSpeeds[i]);
                mOffsets[i].y = sinf(mAccumulatedTime * mYSpeeds[i]);
                mOffsets[i].z = sinf(mAccumulatedTime * mZSpeeds[i]);
            }

        }


        void WaveDeformInstance::onInit(nap::EntityInstance* entity)
        {
            mAmount = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);
            mReach = getParameterManager().addParameterFloat("radius", 1.0, 0.0, 10.0);
            mSpeed = getParameterManager().addParameterFloat("speed", 1.0, 0.0, 10.0);
            mWaveLength = getParameterManager().addParameterFloat("waveLength", 1.0, 0.01, 10.0);
            mAngle = getParameterManager().addParameterFloat("angle", 0.0, 0.0, 360.0);
            mWorldSpace = getParameterManager().addParameterBool("waveInWorldSpace", true);
            
            mEnable = getParameterManager().addParameterBool("enable", false);

        }

        void WaveDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            if(mEnable->mValue)
            {

                glm::vec3 direction = glm::vec3(cosf(glm::radians(mAngle->mValue)), 0., sinf(glm::radians(mAngle->mValue)));
                
                for(int i = 0; i < transforms.size(); i++)
                {
                    
                    glm::vec3 particlePosition;
                    if(mWorldSpace->mValue == false) // need to calculate in object space
                        particlePosition = GlmFunctions::worldSpaceToObjectSpace(transforms[i].mPosition, soundObjectTransform);
                    else
                        particlePosition = transforms[i].mPosition;
                    
                    float projectionOnLine = glm::dot(particlePosition, direction);
                    
                    transforms[i].mPosition.y += sinf(mAccumulatedTime + projectionOnLine * 2 * M_PI / mWaveLength->mValue) * mReach->mValue * mAmount->mValue;
                    
                }
                
            }

        }

        void WaveDeformInstance::update(double deltaTime)
        {
            mAccumulatedTime += deltaTime * mSpeed->mValue;
        }


        void BuzzDeformInstance::onInit(nap::EntityInstance* entity)
        {

            mAmount = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);
            mReach = getParameterManager().addParameterFloat("radius", 1.0, 0.0, 10.0);
            mSpeed = getParameterManager().addParameterFloat("speed", 1.0, 0.0, 10.0);
            
            mEnable = getParameterManager().addParameterBool("enable", false);

        }

        void BuzzDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            if(mEnable->mValue)
            {

                const float pi = math::pi();
                
                glm::vec3 objectSpacePosition;
                            
                for(int i = 0; i < transforms.size() && i < DEFORM_MAX_PARTICLES; i++){
                    
                    objectSpacePosition = GlmFunctions::worldSpaceToObjectSpace(transforms[i].mPosition, soundObjectTransform);
                    objectSpacePosition.y += sinf(mAccumulatedTime + objectSpacePosition.x * pi) * mReach->mValue * mAmount->mValue;
                    objectSpacePosition.z += sinf(mAccumulatedTime + objectSpacePosition.y * pi) * mReach->mValue * mAmount->mValue;
                    objectSpacePosition.x += sinf(mAccumulatedTime + objectSpacePosition.z * pi) * mReach->mValue * mAmount->mValue;
                    
                    transforms[i].mPosition = GlmFunctions::objectSpaceToWorldSpace(objectSpacePosition, soundObjectTransform);
                }
                
            }

        }


        void BuzzDeformInstance::update(double deltaTime)
        {
            mAccumulatedTime += deltaTime * mSpeed->mValue;
        }


        void ShakeDeformInstance::onInit(nap::EntityInstance* entity)
        {

            mAmount = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);
            mReach = getParameterManager().addParameterFloat("radius", 1.0, 0.0, 10.0);
            mDuration = getParameterManager().addParameterFloat("duration", 1.0, 0.0, 10.0);
            mTrigger = getParameterManager().addParameterBool("trigger", false);
            mTrigger->valueChanged.connect(triggerSlot);
            
            mEnable = getParameterManager().addParameterBool("enable", false);

            regenerateTargetOffsets();
            std::copy(std::begin(mTargetOffsets), std::end(mTargetOffsets), std::begin(mOffsets));
            std::copy(std::begin(mTargetOffsets), std::end(mTargetOffsets), std::begin(mPreviousOffsets));

        }

        void ShakeDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {

            if(mEnable->mValue)
            {

                for(int i = 0; i < transforms.size() && i < DEFORM_MAX_PARTICLES; i++){
                    transforms[i].mPosition += mOffsets[i] * mReach->mValue * mAmount->mValue;
                }
                
            }

        }


        void ShakeDeformInstance::update(double deltaTime)
        {

            if(mMoving){

                mFraction += mFractionIncrement * deltaTime;

                if(mFraction >= 1.){

                    for(int i = 0; i < DEFORM_MAX_PARTICLES; i++)
                        mOffsets[i] = mTargetOffsets[i];

                    mMoving = false;
                }
                else{

                    for(int i = 0; i < DEFORM_MAX_PARTICLES; i++)
                        mOffsets[i] = (1.f - mFraction) * mPreviousOffsets[i] + mFraction * mTargetOffsets[i];

                }

            }
        }


        void ShakeDeformInstance::onTrigger(bool)
        {
            regenerateTargetOffsets();
            mMoving = true;
            mFraction = 0.;
            mFractionIncrement = 1. / mDuration->mValue;
            std::copy(std::begin(mOffsets), std::end(mOffsets), std::begin(mPreviousOffsets));

            mTrigger->mValue = false;
        }

        void ShakeDeformInstance::regenerateTargetOffsets()
        {
            for(int i = 0; i < DEFORM_MAX_PARTICLES; i++){
                mTargetOffsets[i] = glm::vec3(math::random<float>(-1., 1.),math::random<float>(-1., 1.),math::random<float>(-1., 1.));
            }
        }


    }

}
