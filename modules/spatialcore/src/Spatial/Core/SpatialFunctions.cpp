#include "SpatialFunctions.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>

// Nap includes
#include <mathutils.h>

// Glm includes
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>


RTTI_BEGIN_CLASS(nap::spatial::GlmFunctions)
    RTTI_FUNCTION("xyzToAngleAxis", &nap::spatial::GlmFunctions::xyzToAngleAxis)
    RTTI_FUNCTION("distance", &nap::spatial::GlmFunctions::distance)
    RTTI_FUNCTION("rotate", &nap::spatial::GlmFunctions::rotate)
    RTTI_FUNCTION("simplex", &nap::spatial::GlmFunctions::simplex)
    RTTI_FUNCTION("lookAtInAngleAxis", &nap::spatial::GlmFunctions::lookAtInAngleAxis)

RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        float GlmFunctions::distance(glm::vec3 a, glm::vec3 b)
        {
            return glm::distance(a,b);
        }

        glm::vec3 GlmFunctions::rotate(glm::vec3 point, glm::vec4 angleAxis)
        {
            return glm::rotate(point, angleAxis[0], glm::vec3(angleAxis[1],angleAxis[2],angleAxis[3]));
        }


        float GlmFunctions::simplex(float t, float y)
        {
            return glm::simplex(glm::vec2(t, y));
        }

        
        glm::vec4 GlmFunctions::xyzToAngleAxis(glm::vec3 xyz)
        {
            glm::quat rotation = glm::quat(glm::vec3(toRadians(xyz[0]), toRadians(xyz[1]), toRadians(xyz[2])));

            float angle = -1. * glm::angle(rotation); // Negate here because we want positive rotation to be clockwise and not counterclockwise
            glm::vec3 axis = glm::axis(rotation);

            return glm::vec4(angle, axis[0], axis[1], axis[2]);
        }


        float GlmFunctions::toRadians(float inDegrees)
        {
            return inDegrees * (M_PI / 180.);
        }
        
        
        glm::vec4 GlmFunctions::compositeAngleAxisRotations(const glm::vec4& rotation, const glm::vec4& angleAxisRotationOffset)
        {
            glm::quat rotationTransformation = glm::angleAxis(angleAxisRotationOffset[0], glm::vec3(angleAxisRotationOffset[1], angleAxisRotationOffset[2], angleAxisRotationOffset[3]));
            glm::quat rotationQuat = glm::angleAxis(rotation[0], glm::vec3(rotation[1],rotation[2],rotation[3]));
            rotationQuat = glm::cross(rotationTransformation, rotationQuat);
            float rotationAngle = glm::angle(rotationQuat);
            glm::vec3 rotationAxis = glm::axis(rotationQuat);
            return glm::vec4(rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
        }
        
        
        glm::vec4 GlmFunctions::lookAtInAngleAxis(glm::vec3 eye, glm::vec3 subject)
        {
            
            if(eye == subject)
                return glm::vec4(0, 0, 1, 0);
            
            // to prevent NaN cases..
            if(eye.x == subject.x)
                eye.x += 0.0001;
            if(eye.y == subject.y)
                eye.y += 0.0001;
            if(eye.z == subject.z)
                eye.z += 0.0001;
                
            glm::mat4x4 m = glm::lookAt(eye, subject, glm::vec3(0,1,0));
            glm::quat q = glm::quat_cast(m);
            float angle = -1. * glm::angle(q); // don't know exactly why I have to negate the angle
            glm::vec3 axis = glm::axis(q);
            return glm::vec4(angle, axis[0], axis[1], axis[2]);

        }

        
        float Functions::distanceScale(float value, float inputMin, float inputMax, float outputMin, float outputMax)
        {
            
            if(inputMax - inputMin == 0)
                return 0;
            
            float proportion = (value - inputMin) / (inputMax - inputMin);
            
            return outputMin + (proportion * (outputMax - outputMin));
            
        }
        
        float Functions::distanceCurve(float input, float curvature, bool up)
        {
            if(up){
                if(curvature < 0)
                    return math::power<float>(input, distanceScale(curvature, -1, 0, 10, 1));
                else
                    return math::power<float>(input, distanceScale(curvature, 0, 1, 1, 0.1));
            }
            else{
                if(curvature < 0)
                    return math::power<float>(1. - input, distanceScale(curvature, -1, 0, 10, 1));
                else
                    return math::power<float>(1. - input, distanceScale(curvature, 0, 1, 1, 0.1));
            }
        }
        
        float Functions::getFrequencyForFraction(float fraction)
        {
            float elog = fraction * LN20K + (1. - fraction) * LN20;
            return std::exp(elog);
        }

        
        
        glm::vec3 GlmFunctions::shapeSpaceToWorldSpace(const glm::vec3& shapeSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            glm::vec3 particlePosition = GlmFunctions::rotate(shapeSpacePosition, soundObjectTransform.mRotation);
            particlePosition += soundObjectTransform.mPosition;
            return particlePosition;
        }

        
        glm::vec3 GlmFunctions::worldSpaceToShapeSpace(const glm::vec3& worldSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            glm::vec3 shapeSpacePosition = worldSpacePosition - soundObjectTransform.mPosition;
            shapeSpacePosition = GlmFunctions::rotate(shapeSpacePosition, glm::vec4(-soundObjectTransform.mRotation[0], soundObjectTransform.mRotation[1], soundObjectTransform.mRotation[2], soundObjectTransform.mRotation[3]));
            return shapeSpacePosition;
        }
        
        
        glm::vec3 GlmFunctions::objectSpaceToWorldSpace(const glm::vec3& objectSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            return shapeSpaceToWorldSpace(objectSpaceToShapeSpace(objectSpacePosition, soundObjectTransform), soundObjectTransform);
        }
        
        glm::vec3 GlmFunctions::worldSpaceToObjectSpace(const glm::vec3& worldSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            return shapeSpaceToObjectSpace(worldSpaceToShapeSpace(worldSpacePosition, soundObjectTransform), soundObjectTransform);
        }


        glm::vec3 GlmFunctions::objectSpaceToShapeSpace(const glm::vec3& objectSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            return objectSpacePosition * 0.5f * soundObjectTransform.mScale;
        }
        

        glm::vec3 GlmFunctions::shapeSpaceToObjectSpace(const glm::vec3& shapeSpacePosition, const SpatialTransform& soundObjectTransform)
        {
            
            glm::vec3 outVector(0.);
            
            // divide by zero fixes
            if(soundObjectTransform.mScale.x == 0.f)
                outVector.x = 0.f;
            else
                outVector.x = shapeSpacePosition.x / (0.5f * soundObjectTransform.mScale.x);
            
            if(soundObjectTransform.mScale.y == 0.f)
                outVector.y = 0.f;
            else
                outVector.y = shapeSpacePosition.y / (0.5f * soundObjectTransform.mScale.y);
            
            if(soundObjectTransform.mScale.z == 0.f)
                outVector.z = 0.f;
            else
                outVector.z = shapeSpacePosition.z / (0.5f * soundObjectTransform.mScale.z);
            
            return outVector;
        }

        
    }

}
