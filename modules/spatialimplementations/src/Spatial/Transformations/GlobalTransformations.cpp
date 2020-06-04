#include "GlobalTransformations.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialFunctions.h>

// NAP includes
#include <mathutils.h>

// GLM includes
#include <glm/gtx/rotate_vector.hpp>


RTTI_DEFINE_CLASS(nap::spatial::PlodeTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PlodeTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::PlodeScaleTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PlodeScaleTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::StretchTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::StretchTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::StretchScaleTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::StretchScaleTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::RotationTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::RotationTransformationInstance)
RTTI_END_CLASS


namespace nap
{
    
    namespace spatial
    {
        
        void PlodeTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mPlode = getParameterManager().addParameterVec3("", glm::vec3(1.0), -100., 100.);
        }
        
        
        void PlodeTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            position -= getAnchorPoint();
            position *= mPlode->mValue;
            position += getAnchorPoint();
        }

        
        void PlodeScaleTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mPlodeScale = getParameterManager().addParameterFloat("", 1.0, 0., 10.);
        }
        
        
        void PlodeScaleTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            position -= getAnchorPoint();
            position *= mPlodeScale->mValue;
            position += getAnchorPoint();
        }

        
        void StretchTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mStretch = getParameterManager().addParameterVec3("", glm::vec3(1.0), -100., 100.);
        }
        
        
        void StretchTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            position -= getAnchorPoint();
            position *= mStretch->mValue;
            scale *= mStretch->mValue;
            position += getAnchorPoint();
        }
        
        
        void StretchScaleTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mStretchScale = getParameterManager().addParameterFloat("", 1.0, 0., 10.);
        }
        
        
        void StretchScaleTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            position -= getAnchorPoint();
            position *= mStretchScale->mValue;
            scale *= mStretchScale->mValue;
            position += getAnchorPoint();
        }
        
        
        void RotationTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            mRotation = getParameterManager().addParameterVec3("", glm::vec3(0.0), -360.0, +360.0);
        }
        
        
        void RotationTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            // rotate position
            position = GlmFunctions::rotate(position, GlmFunctions::xyzToAngleAxis(mRotation->mValue));

            // rotate orientation
            rotation = GlmFunctions::compositeAngleAxisRotations(rotation, GlmFunctions::xyzToAngleAxis(mRotation->mValue));
        }
        
        
    }
    
}
