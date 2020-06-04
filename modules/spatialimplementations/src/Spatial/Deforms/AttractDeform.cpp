
#include "AttractDeform.h"

// Spatial includes
#include <Spatial/Core/SpatialFunctions.h>
#include <Spatial/Core/ParameterComponent.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::AttractDeformInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::AttractDeform)


namespace nap
{
    namespace spatial
    {
        
        static const float TWO_TIMES_SQRT3 = 3.4641016151; // diagonal length of cube
        
        
        void AttractDeformInstance::onInit(nap::EntityInstance* entity)
        {
            
            mType = getParameterManager().addParameterOptionList("type", "point", {"point","line","plane"});
            
            mCurve = getParameterManager().addParameterFloat("curve", 0., 0., 1.);
            
            mAmount = getParameterManager().addParameterFloat("amount", 0., -1., 1.);
            mOffset = getParameterManager().addParameterVec3("offset", glm::vec3(0.), -1.0, 1.0);
            
            mAzimuth = getParameterManager().addParameterFloat("horizontal", 0., -360., 360.0);
            mElevation = getParameterManager().addParameterFloat("vertical", 0., -90., 90.);
            
            mTwist = getParameterManager().addParameterFloat("twist", 0., 0., 1.);
            
            mModulations.reserve(3);
            mModulations.emplace_back(Modulation(getParameterManager(), "modulation/x"));
            mModulations.emplace_back(Modulation(getParameterManager(), "modulation/y"));
            mModulations.emplace_back(Modulation(getParameterManager(), "modulation/z"));
            
            mEnable = getParameterManager().addParameterBool("enable", false);
            
        }
                
        void AttractDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            if(mEnable->mValue)
            {
            
                for(auto& transform : transforms)
                {
                    glm::vec3 position = GlmFunctions::worldSpaceToObjectSpace(transform.mPosition, soundObjectTransform);

                    glm::vec3 direction(0.);
                    float distance = 0.;
                    
                    glm::vec3 offset = mOffset->mValue + glm::vec3(mModulations[0].getValue(), mModulations[1].getValue(), mModulations[2].getValue());

                    
                    
                    // 1. Calculate direction and distance based on settings.
                    if(mType->getValue() == 0) // point
                    {
                        direction = glm::normalize(offset - position);
                        distance = glm::distance(offset, position);
                    }
                    else if(mType->getValue() == 1) // line
                    {
                        
                        // find distance and closest point on line based on theta&phi (algorithm by Claire.. could be replaced with glm call?)
                        float theta = GlmFunctions::toRadians(mElevation->mValue - 90.f);
                        float phi = GlmFunctions::toRadians(mAzimuth->mValue);
                        glm::vec3 lineVector(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
                        glm::vec3 particleToAttractionPoint = offset - position;
                        float scalar = glm::dot(particleToAttractionPoint, lineVector);
                        glm::vec3 HA = scalar * lineVector; // projected particleToAttractionPoint on lineVector.
                        glm::vec3 particleToLine = particleToAttractionPoint - HA; // vector from particle to line. Attraction direction.
                        
                        direction = glm::normalize(particleToLine);
                        distance = glm::length(particleToLine);

                        
                    }
                    else // plane
                    {
                        
                        float theta = GlmFunctions::toRadians(mElevation->mValue - 90.f);
                        float phi = GlmFunctions::toRadians(mAzimuth->mValue);
                        glm::vec3 planeNormal(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
                        glm::vec3 particleToAttractionPoint = offset - position;
                        float scalar = glm::dot(particleToAttractionPoint, planeNormal);
                        glm::vec3 PH = planeNormal * scalar;

                        distance = std::abs(scalar);
                        direction = glm::normalize(PH);
                        
                    }
                    
                    
                    // 2. Apply curve and offset position
                    if(distance > 0.00001 && (mAmount->mValue != 0.f)){
                        float amplitude = Functions::distanceCurve(distance / TWO_TIMES_SQRT3, mCurve->mValue, true) * TWO_TIMES_SQRT3;
                        
                        // add twist offset
                        float twist = 2.f * M_PI * (distance / TWO_TIMES_SQRT3);
                        glm::vec3 xAxis;
                        if(direction != glm::vec3(1., 0., 0.))
                            xAxis = glm::cross(glm::vec3(1., 0., 0.), direction);
                        else
                            xAxis = glm::cross(glm::vec3(0., 1., 0.), direction);
                        glm::vec3 yAxis = glm::cross(offset, xAxis);
                        glm::vec3 twistOffset = sinf(twist) * yAxis + cosf(twist) * xAxis;
                        
                        position = position + mAmount->mValue * amplitude * direction;
                        position += twistOffset * mTwist->mValue;
                        transform.mPosition = GlmFunctions::objectSpaceToWorldSpace(position, soundObjectTransform);
                        
                    }
                    
                }
                
            }
            
        }
        
        
        void AttractDeformInstance::update(double deltaTime)
        {
            mAccumulatedTime += deltaTime;
            
            for(auto& mod : mModulations)
                mod.update(deltaTime, mAccumulatedTime);

        }

    }
    
}
