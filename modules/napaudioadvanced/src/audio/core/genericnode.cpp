#include "genericnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GenericNode)
    RTTI_FUNCTION("getInputCount", &nap::audio::GenericNode::getInputCount)
    RTTI_FUNCTION("getOutputCount", &nap::audio::GenericNode::getOutputCount)
    RTTI_FUNCTION("getInput", &nap::audio::GenericNode::getInput)
    RTTI_FUNCTION("getOutput", &nap::audio::GenericNode::getOutput)
    RTTI_FUNCTION("hasParameter", &nap::audio::GenericNode::hasParameter)
    RTTI_FUNCTION("getParameterValue", &nap::audio::GenericNode::getParameterValue)
    RTTI_FUNCTION("setParameterValue", &nap::audio::GenericNode::setParameterValue)
    RTTI_FUNCTION("setParameterNormalizedValue", &nap::audio::GenericNode::setParameterNormalizedValue)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
        
        std::vector<std::string> GenericNode::getParameterNames() const
        {
            std::vector<std::string> result;
            for (auto& pair : mParameters)
                result.emplace_back(pair.first);
            return result;
        }

        
        bool GenericNode::setParameterValue(const std::string& name, float value)
        {
            auto it = mParameters.find(name);
            if (it == mParameters.end())
                return false;
            else {
                *it->second.mZone = value;
                return true;
            }
        }
        
        
        bool GenericNode::setParameterNormalizedValue(const std::string& name, float value)
        {
            auto it = mParameters.find(name);
            if (it == mParameters.end())
                return false;
            else {
                Parameter& parameter = it->second;
                *parameter.mZone = parameter.mMin + value * (parameter.mMax - parameter.mMin);
                return true;
            }
        }

        
        
        float* GenericNode::getParameterValue(const std::string& name) const
        {
            auto it = mParameters.find(name);
            if (it == mParameters.end())
                return nullptr;
            else
                return it->second.mZone;
        }


        
        void GenericNode::setInputCount(unsigned int count)
        {
            mInputs.resize(count);
            for (auto& input : mInputs)
                input = std::make_unique<InputPin>(this);
        }
        
    
        void GenericNode::setOutputCount(unsigned int count)
        {
            mOutputs.resize(count);
            for (auto& output : mOutputs)
                output = std::make_unique<OutputPin>(this);
        }
        
        
        void GenericNode::addParameter(const std::string& name, float* zone, float min, float max)
        {
            Parameter parameter;
            parameter.mZone = zone;
            parameter.mMin = min;
            parameter.mMax = max;
            mParameters[name] = parameter;
        }

    }
    
}
