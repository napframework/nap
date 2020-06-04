
#include "ParameterUtilities.h"

#include "ParameterTypes.h"

#include <SDL.h>
#include <numeric>

namespace nap
{
    namespace spatial
    {
        
        void exportParametersToClipboard(std::vector<ParameterGroup*> parameterGroups)
        {
            // create an overview of all of the registered parameters and copy it to clipboard
            
            std::string outputString = "";
            
            outputString +=
            "ID"
            "\tType"
            "\tValue"
            "\tRange Minimum"
            "\tRange Maximum"
            "\tEnumeration Values";
            
            for(auto& parameterGroup : parameterGroups)
            {
                // sort parameters
                auto parameters = parameterGroup->mParameters;
                std::sort(parameters.begin(), parameters.end(), [](const ResourcePtr<Parameter> & a, const ResourcePtr<Parameter> & b) { return a->mID < b->mID; });
                
                for (auto parameter : parameters)
                {
                    // 1. ID
                    outputString += "\n" + parameter->mID;
                    
                    // 2. Type
                    std::string typeString = "-";
                    std::string valueString = "-";
                    std::string minString = "-";
                    std::string maxString = "-";
                    std::string optionsString = "-";
                    
                    auto type = parameter->get_type();
                    
                    if(type == RTTI_OF(ParameterFloat))
                    {
                        typeString = "float";
                        auto p = rtti_cast<ParameterFloat>(parameter.get());
                        valueString = std::to_string(p->mValue);
                        minString = std::to_string(p->mMinimum);
                        maxString = std::to_string(p->mMaximum);
                    }
                    else if(type == RTTI_OF(ParameterDouble))
                    {
                        typeString = "double";
                        auto p = rtti_cast<ParameterDouble>(parameter.get());
                        valueString = std::to_string(p->mValue);
                        minString = std::to_string(p->mMinimum);
                        maxString = std::to_string(p->mMaximum);
                    }
                    else if(type == RTTI_OF(ParameterInt))
                    {
                        typeString = "int";
                        auto p = rtti_cast<ParameterInt>(parameter.get());
                        valueString = std::to_string(p->mValue);
                        minString = std::to_string(p->mMinimum);
                        maxString = std::to_string(p->mMaximum);
                    }
                    else if(type == RTTI_OF(ParameterBool))
                    {
                        typeString = "bool";
                        auto p = rtti_cast<ParameterBool>(parameter.get());
                        valueString = p->mValue ? "True" : "False";
                    }
                    else if(type == RTTI_OF(ParameterVec2))
                    {
                        typeString = "vec2";
                        auto p = rtti_cast<ParameterVec2>(parameter.get());
                        valueString = std::to_string(p->mValue.x) + " " + std::to_string(p->mValue.y);
                        minString = std::to_string(p->mMinimum);
                        maxString = std::to_string(p->mMaximum);
                    }
                    else if(type == RTTI_OF(ParameterVec3))
                    {
                        typeString = "vec3";
                        auto p = rtti_cast<ParameterVec3>(parameter.get());
                        valueString = std::to_string(p->mValue.x) + " " + std::to_string(p->mValue.y) + " " + std::to_string(p->mValue.z);
                        minString = std::to_string(p->mMinimum);
                        maxString = std::to_string(p->mMaximum);
                    }
                    else if(type == RTTI_OF(ParameterOptionList))
                    {
                        typeString = "option list";
                        auto p = rtti_cast<ParameterOptionList>(parameter.get());
                        valueString = p->getOptionName();
                        auto options = p->getOptions();
                        optionsString = std::accumulate(
                                                        std::next(options.begin()),
                                                        options.end(),
                                                        options[0],
                                                        [](std::string a, std::string b) {
                                                            return a + ", " + b;
                                                        }
                                                        );
                    }
                    else if(type == RTTI_OF(ParameterString))
                    {
                        typeString = "string";
                        auto p = rtti_cast<ParameterString>(parameter.get());
                        valueString = p->mValue;
                    }
                    
                    outputString += "\t" + typeString + "\t" + valueString + "\t" + minString + "\t" + maxString + "\t" + optionsString;
                }
            }
            
            SDL_SetClipboardText(outputString.c_str());
            
        }
        
    }
}
