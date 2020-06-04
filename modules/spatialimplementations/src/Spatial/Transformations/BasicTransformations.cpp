#include "BasicTransformations.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialFunctions.h>

// NAP includes
#include <numeric>

RTTI_DEFINE_CLASS(nap::spatial::InputPositionTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InputPositionTransformationInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::InputDimensionsTransformation)
RTTI_PROPERTY("DefaultValue", &nap::spatial::InputDimensionsTransformation::mDefaultValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InputDimensionsTransformationInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::InputScaleTransformation)
    RTTI_PROPERTY("DefaultValue", &nap::spatial::InputScaleTransformation::mDefaultValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InputScaleTransformationInstance)
RTTI_END_CLASS


RTTI_DEFINE_CLASS(nap::spatial::OrientationTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::OrientationTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::PositionRotationTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PositionRotationTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::InvertTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InvertTransformationInstance)
RTTI_END_CLASS


namespace nap {
    namespace spatial {
        
        void InputPositionTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mInputPosition = getParameterManager().addParameterVec3("", glm::vec3(0.0), -100.f, 100.f);
        }
        
        
        void InputPositionTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            position += mInputPosition->mValue;
        }

        
        void InputDimensionsTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mInputDimensions = getParameterManager().addParameterVec3("", glm::vec3(getTransformation<InputDimensionsTransformation>()->mDefaultValue), -100.f, 100.f);
        }
        
        
        void InputDimensionsTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            scale *= mInputDimensions->mValue;
        }
        
        
        void InputScaleTransformationInstance::onInit(nap::EntityInstance* entity)
        {
             mInputScale = getParameterManager().addParameterFloat("", getTransformation<InputScaleTransformation>()->mDefaultValue, 0.0, 100.0);
        }
        
        
        void InputScaleTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            scale *= mInputScale->mValue;
        }
        
        
        void OrientationTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mInputRotation = getParameterManager().addParameterVec3("", glm::vec3(0.0), -360., 360.);
            mRotationMode = getParameterManager().addParameterOptionList("mode", "fixed", {"fixed", "movement", "center" });
            
            // pre-fill pastPositions vector with (0,0,0)
            for(int i = 0; i < NUM_LAST_POSITIONS; i++)
                mLastPositions[i] = glm::vec3(0,0,0);
            
        }
        
        
        void OrientationTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            
            int mode = mRotationMode->getValue();
            
            if(mode == 0){
                rotation = GlmFunctions::compositeAngleAxisRotations(rotation, GlmFunctions::xyzToAngleAxis(mInputRotation->mValue));
            }
            else if(mode == 1){
                // set last position and increment write pointer.
                mLastPositions[mLastPositionsIndex] = position;
                mLastPositionsIndex = (mLastPositionsIndex + 1) % mLastPositions.size();
                
                // calculate average position
                glm::vec3 averagePosition = std::accumulate(mLastPositions.begin(), mLastPositions.end(), glm::vec3(0.)) / (float)mLastPositions.size();
                
                // calculate orientation
                // NOTE: This is the position at this point in the chain. This forces us to put the OrientationTransformation at the end of the chain.
                rotation = GlmFunctions::compositeAngleAxisRotations(rotation, GlmFunctions::lookAtInAngleAxis(averagePosition, position));
            }
            else if(mode == 2){
                rotation = GlmFunctions::compositeAngleAxisRotations(rotation, GlmFunctions::lookAtInAngleAxis(position, getAnchorPoint()));
            }
            
        }
        

        void PositionRotationTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mPositionRotation = getParameterManager().addParameterVec3("", glm::vec3(0.0), -360, 360);
        }
        
        
        void PositionRotationTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            
            auto anchorPoint = getAnchorPoint();
            
            position -= anchorPoint;
            
            position = GlmFunctions::rotate(position, GlmFunctions::xyzToAngleAxis(mPositionRotation->mValue));
            
            position += anchorPoint;
            
        }
      
        
        void InvertTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mInvertX = getParameterManager().addParameterBool("x", false);
            mInvertY = getParameterManager().addParameterBool("y", false);
            mInvertZ = getParameterManager().addParameterBool("z", false);
        }
        
        
        void InvertTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            
            auto anchorPoint = getAnchorPoint();
            
            position -= anchorPoint;
            
            if(mInvertX->mValue)
                position.x = -position.x;
            if(mInvertY->mValue)
                position.y = -position.y;
            if(mInvertZ->mValue)
                position.z = -position.z;
            
            position += anchorPoint;
            
        }

    }
}




