#include "ExternalTransformation.h"

// Spatial includes
#include <Spatial/Transformation/TransformationChainComponent.h>
#include <Spatial/Core/ParameterComponent.h>

RTTI_DEFINE_CLASS(nap::spatial::ExternalTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ExternalTransformationInstance)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::spatial::SwitchExternalTransformation)
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SwitchExternalTransformationInstance)
RTTI_END_CLASS

namespace nap {
    namespace spatial {
        
        
        void ExternalTransformationInstance::onInit(nap::EntityInstance* entity)
        {
        }
        
        void ExternalTransformationInstance::createEnableParameter(bool defaultValue, bool shared)
        {
            if(shared)
                mEnable = getParameterManager().addParameterBool(getName() + "/enable", defaultValue, true);
            else
                mEnable = getParameterManager().addParameterBool("enable", defaultValue, false);
        }
        
        void ExternalTransformationInstance::setTransformationChain(TransformationChainComponentInstance* chain)
        {
            mExternalTransformationChainComponent = chain;
        }
        
        void ExternalTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            if(mExternalTransformationChainComponent != nullptr && mEnable->mValue)
                mExternalTransformationChainComponent->apply(position, scale, rotation);
        }
        
        void SwitchExternalTransformationInstance::setTransformationChains(std::vector<TransformationChainComponentInstance*> chains, std::vector<std::string> names)
        {
            
            assert(names.size() == chains.size());
            
            mExternalTransformationChainComponents = chains;
            
            std::vector<std::string> options = names;
            options.insert(options.begin(), "off");
            
            mSwitch = getParameterManager().addParameterOptionList("", "off", options);
            
        }
        
        void SwitchExternalTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            
            if(!mExternalTransformationChainComponents.empty() && mSwitch->getValue() != 0 ){
                mExternalTransformationChainComponents[mSwitch->getValue() - 1]->apply(position, scale, rotation);
            }
        }
        
    }
}
