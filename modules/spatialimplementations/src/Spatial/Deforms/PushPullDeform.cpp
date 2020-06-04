#include "PushPullDeform.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>

// NAP includes
#include <mathutils.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PushPullDeformInstance)
RTTI_END_CLASS
RTTI_DEFINE_CLASS(nap::spatial::PushPullDeform)

namespace nap
{
    namespace spatial
    {
        
        
        void PushPullDeformInstance::onInit(nap::EntityInstance* entity)
        {
            
            mEnable = getParameterManager().addParameterBool("enable", false);
            mAmount = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);

            mPush = getParameterManager().addParameterBool("push", true);
            mTheta = getParameterManager().addParameterFloat("theta", 0.0, -2.0, 2.0);
            mPhi = getParameterManager().addParameterFloat("phi", 0.0, -2.0, 2.0);
            
            mMode = getParameterManager().addParameterOptionList("mode", "symmetrical", {"straight", "smooth", "symmetrical"});
            
            mTension = getParameterManager().addParameterOptionList("contact", "line", {"point", "line", "plane"});
            
            mIntensity = getParameterManager().addParameterFloat("intensity", 1.0, 0.0, 10.0);
            mWidth = getParameterManager().addParameterFloat("width", 1.0, 0.01, 10.0);
            
            mDeformation = getParameterManager().addParameterOptionList("deformation", "gaussian", {"steady", "linear", "polynomial", "gaussian", "exponential"});
            
            mTwist = getParameterManager().addParameterFloat("twist", 0.0, 0.0, 10.0);
            
            mPosition = getParameterManager().addParameterVec3("position", glm::vec3(0.0), -10.0, 10.0);
            
        }
        
        void PushPullDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            if(mEnable->mValue)
            {
            
                for(int i = 0; i < transforms.size(); i++){
                    
                    glm::vec3& particlePosition = transforms[i].mPosition;
                    
                    glm::vec3 direction(sin(mTheta->mValue) * cos(mPhi->mValue), cos(mTheta->mValue), sin(mTheta->mValue) * sin(mPhi->mValue));
                    
                    glm::vec3 PA = mPosition->mValue - particlePosition;
                    float scalar = glm::dot(PA, direction);
                    
                    // calculate distance
                    
                    float distance = 0.;
                    
                    if(mTension->getValue() == 0){ // point
                        distance = glm::length(PA);
                    }
                    else if(mTension->getValue() == 1){ // line
                        // Casi: H is the projection of particlePosition on the line from A with direction 'direction'.
                        glm::vec3 HA = direction * -1.f * scalar;
                        glm::vec3 PH = PA - HA;
                        distance = glm::length(PH);
                    }
                    else{ // plane
                        distance = std::abs(scalar);
                    }
                    
                    
                    // Claire: AMPLITUDE DEFORM: Here, the deformation is maximal at distance = 0, worth alpha, while beta is the width where significant
                    float amp = 0;
                    if(distance > 0.)
                        amp = deformAmp(distance, mDeformation->getValue(), mIntensity->mValue, mWidth->mValue);
                    
                    
                    int sign;
                    if(mMode->getValue() == 0){ // straight
                        if(mPush->mValue)
                            sign = scalar > 0.;
                        else
                            sign = scalar < 0.;
                    }
                    else if(mMode->getValue() == 1){ // smooth
                        sign = 1;
                        
                    }
                    else{ // symmetrical
                        if(mPush->mValue){
                            if(scalar > 0.)
                                sign = -1;
                            else
                                sign = 1;
                            
                            // Casi: why is this here?? A mistake? What does this even have to do with 'symmetrical'..
                            if(mPush->mValue){
                                if(std::abs(scalar) < amp)
                                    amp = std::abs(scalar);
                            }
                            
                        }
                        else{
                            if(scalar > 0.)
                                sign = 1;
                            else
                                sign = -1;
                        }
                    }
                    
                    // push & pull
                    particlePosition += mAmount->mValue * sign * amp * direction;
                    
                    // twist
                    float angleTwist = 2 * math::pi() * mTwist->mValue * (1. - (distance / mWidth->mValue));
                    if(angleTwist < 0.)
                        angleTwist = 0.;
                    
                    particlePosition = (1.f - mAmount->mValue) * particlePosition + mAmount->mValue * twist(particlePosition, angleTwist, mPosition->mValue, direction);
                    
                    // ....
                }
                
            }
            
        }
        
        
        // Claire:
        //# Return the deformation amplitude which depends on the (positive) input 'distance', with different functions according to typeD.
        //# This deformation is maximale when 'distance' is 0, and its value is then alpha, and then is significant around beta
        //# This deformation depends on the parameters given (like scopeD, and amountD).
        //# xD non zero here, same for scopeD
        //# Other types could also be implemented. Notably with exponent
        
        float PushPullDeformInstance::deformAmp(float distance, int type, float alpha, float beta)
        {
            if(type == 0){ // steady
                if(distance <= beta)
                    return alpha;
            }
            else if(type == 1){ // linear
                if(distance <= beta)
                    return alpha * (1. - (distance / beta));
            }
            else if(type == 2){ // polynomial
                if(distance <= beta)
                    return alpha * (1. - (distance / beta) * (1. - (distance / beta)));
            }
            else if(type == 3){ // gaussian
                return alpha * exp(- ((distance / beta) * (distance / beta)));
            }
            else if(type == 4){ // exponential
                return alpha * exp((-distance) / beta);
            }
            return 0.;
        }
        
        
        glm::vec3 PushPullDeformInstance::twist(glm::vec3 pointR, float theta, glm::vec3 pointC, glm::vec3 dir)
        {
            float x = pointR.x;
            float y = pointR.y;
            float z = pointR.z;
            float a = pointC.x;
            float b = pointC.y;
            float c = pointC.z;
            float u = dir.x;
            float v = dir.y;
            float w = dir.z;
            
            float newX = ( a * ( v * v + w * w) - u * ( b * v + c * w - u * x - v * y - w * z)) * (1 - cos(theta)) + x * cos(theta) + (- c * v + b * w - w * y + v * z) * sin(theta);
            float newY = ( b * ( u * u + w * w) - v * ( a * u + c * w - u * x - v * y - w * z)) * (1 - cos(theta)) + y * cos(theta) + ( c * u - a * w + w * x - u * z) * sin(theta);
            float newZ = ( c * ( v * v + u * u) - w * ( b * v + a * u - u * x - v * y - w * z)) * (1 - cos(theta)) + z * cos(theta) + (- b * u + a * v - v * x + u * y) * sin(theta);
            
            return glm::vec3(newX, newY, newZ);
            
        }
        
    }
}
