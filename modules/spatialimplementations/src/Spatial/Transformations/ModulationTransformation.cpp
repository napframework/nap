#include "ModulationTransformation.h"

#include <math.h>
#include <mathutils.h>

RTTI_DEFINE_CLASS(nap::spatial::ModulationTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ModulationTransformationInstance)
RTTI_END_CLASS


namespace nap {
    namespace spatial {

        
        void ModulationTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            
            mEnable = getParameterManager().addParameterBool("enable", false);
            
            mSpeed = getParameterManager().addParameterFloat("speed", 0., 0., 20.);
            mScale = getParameterManager().addParameterFloat("scale", 1., 0., 100.);
            
            mModulations.emplace_back(Modulation(getParameterManager(), "x"));
            mModulations.emplace_back(Modulation(getParameterManager(), "y"));
            mModulations.emplace_back(Modulation(getParameterManager(), "z"));
            
        }
        
        void ModulationTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            if(mEnable->mValue)
                position += mOffset;
        }
        
        
        void ModulationTransformationInstance::update(double deltaTime)
        {
            
            float speedAdjustedDelta = deltaTime * mSpeed->mValue;
            mAccumulatedTime += speedAdjustedDelta;
            
            for(auto& mod : mModulations)
                mod.update(speedAdjustedDelta, mAccumulatedTime);
            
            mOffset = glm::vec3(mModulations[0].getValue(), mModulations[1].getValue(), mModulations[2].getValue()) * mScale->mValue;
            
        }
        
    }
}
