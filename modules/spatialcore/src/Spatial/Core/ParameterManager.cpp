#include "ParameterManager.h"

RTTI_BEGIN_CLASS(nap::ParameterManager)
    RTTI_FUNCTION("addParameterFloat", &nap::ParameterManager::addParameterFloat);
    RTTI_FUNCTION("addParameterInt", &nap::ParameterManager::addParameterInt);
    RTTI_FUNCTION("addParameterVec3", &nap::ParameterManager::addParameterVec3);
    RTTI_FUNCTION("addParameterBool", &nap::ParameterManager::addParameterBool);
    RTTI_FUNCTION("addParameterString", &nap::ParameterManager::addParameterString);
    RTTI_FUNCTION("addParameterOptionList", &nap::ParameterManager::addParameterOptionList);
RTTI_END_CLASS

namespace nap {
    
    void ParameterManager::init(ParameterComponentInstance& parameterComponentInstance, std::string prefix, std::string sharedParameterPrefix)
    {
        mParameterComponent = &parameterComponentInstance;
        mPrefix = prefix;
        mSharedParameterPrefix = sharedParameterPrefix == "" ? "" : sharedParameterPrefix + "/";
    }
    
    
    ParameterFloat* ParameterManager::addParameterFloat(const std::string& name, float defaultValue, float min, float max, bool shared)
    {
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterFloat>(mSharedParameterPrefix + name);
            if (param == nullptr)
                param = &mParameterComponent->addParameterFloat(mSharedParameterPrefix + name, defaultValue, min, max);
            return param;
        }
        
        return &mParameterComponent->addParameterFloat(prefixName(name), defaultValue, min, max);
    }
    
    
    ParameterInt* ParameterManager::addParameterInt(const std::string& name, int defaultValue, int min, int max, bool shared)
    {
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterInt>(mSharedParameterPrefix + name);
            if (param == nullptr)
                param = &mParameterComponent->addParameterInt(mSharedParameterPrefix + name, defaultValue, min, max);
            return param;
        }
        
        return &mParameterComponent->addParameterInt(prefixName(name), defaultValue, min, max);
    }
    
    
    ParameterVec3* ParameterManager::addParameterVec3(const std::string& name, const glm::vec3& defaultValue, float min, float max, bool shared)
    {
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterVec3>(mSharedParameterPrefix + name);
            if (param == nullptr)
                param = &mParameterComponent->addParameterVec3(mSharedParameterPrefix + name, defaultValue, min, max);
            return param;
        }
        
        return &mParameterComponent->addParameterVec3(prefixName(name), defaultValue, min, max);
    }
    
    
    ParameterBool* ParameterManager::addParameterBool(const std::string& name, bool defaultValue, bool shared)
    {
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterBool>(mSharedParameterPrefix + name);
            if (param == nullptr)
                param = &mParameterComponent->addParameterBool(mSharedParameterPrefix + name, defaultValue);
            return param;
        }
        
        auto param = &mParameterComponent->addParameterBool(prefixName(name), defaultValue);
        return param;
    }
    
    
    
    ParameterString* ParameterManager::addParameterString(const std::string& name, const std::string& defaultValue, bool shared)
    {
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterString>(mSharedParameterPrefix + name);
            if (param == nullptr)
                param = &mParameterComponent->addParameter<ParameterString>(mSharedParameterPrefix + name, defaultValue);
            return param;
        }
        
        auto param = &mParameterComponent->addParameter<ParameterString>(prefixName(name), defaultValue);
        return param;
        
    }
    
    ParameterOptionList* ParameterManager::addParameterOptionList(const std::string& name, const std::string& defaultValue, const std::vector<std::string>& options, bool shared)
    {
        // find default value
        int index = std::distance(options.begin(), find(options.begin(), options.end(), defaultValue));
        if(index >= options.size())
            index = 0;
        
        if (shared)
        {
            auto param = mParameterComponent->findParameter<ParameterOptionList>(mSharedParameterPrefix + name);
            if (param == nullptr)
            {
                param = &mParameterComponent->addParameter<ParameterOptionList>(mSharedParameterPrefix + name, 0);
                param->setOptions(options);
                param->setValue(index);
            }
            return param;
        }
        
        auto param = &mParameterComponent->addParameter<ParameterOptionList>(prefixName(name), 0);
        param->setOptions(options);
        param->setValue(index);
        
        return param;
    }
    
    
    Parameter* ParameterManager::getExternalParameter(const std::string& name)
    {
        return mParameterComponent->findParameter(name);
    }
    
    
    std::string ParameterManager::prefixName(const std::string& name) const
    {
        if (name.empty())
            return mPrefix;
        
        if (mPrefix.empty())
            return name;
        
        return mPrefix + "/" + name;
    }
    
}
