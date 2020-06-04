#pragma once

#include "ParameterComponent.h"

namespace nap {
    
    /**
     * Class that can create parameters with a certain prefix.
     */
    class NAPAPI ParameterManager
    {
        RTTI_ENABLE()
        
    public:
        ParameterManager() { }
        
        void init(ParameterComponentInstance& parameterComponentInstance, std::string prefix, std::string sharedParameterPrefix);
        
        /**
         * Adds a float parameter.
         * If 'shared' is true, it creates a parameter that is shared with other shapes. (for example, all of the 'theworks' shapes have a 'hollow' parameter.
         */
        ParameterFloat* addParameterFloat(const std::string& name, float defaultValue, float min, float max, bool shared = false);
        
        /**
         * Adds an int parameter.
         */
        ParameterInt* addParameterInt(const std::string& name, int defaultValue, int min, int max, bool shared = false);
        
        /**
         * Adds a vec3 parameter.
         */
        ParameterVec3* addParameterVec3(const std::string& name, const glm::vec3& defaultValue, float min, float max, bool shared = false);
        
        /**
         * Adds a bool parameter.
         */
        ParameterBool* addParameterBool(const std::string& name, bool defaultValue, bool shared = false);
        
        /**
         * Adds a string parameter.
         */
        ParameterString* addParameterString(const std::string& name, const std::string& defaultValue, bool shared = false);
        
        /**
         * Adds an option list parameter.
         */
        ParameterOptionList* addParameterOptionList(const std::string& name, const std::string& defaultValue, const std::vector<std::string>& options, bool shared = false);
        
        /**
         * Finds an already existing parameter by its name.
         */
        Parameter* getExternalParameter(const std::string& name);
        
        template <typename ParameterType>
        ParameterType* getExternalParameter(const std::string& name)
        {
            return mParameterComponent->findParameter<ParameterType>(name);
        }

        const std::string& getNamePrefix() const { return mPrefix; }
        
    private:
        std::string prefixName(const std::string& name) const;
        
        ParameterComponentInstance* mParameterComponent = nullptr;
        std::string mPrefix; ///< Prefix of normal parameters.
        std::string mSharedParameterPrefix; ///< Prefix of shared parameters.
        
    };
    
}
