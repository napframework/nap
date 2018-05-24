#pragma once

// Nap includes
#include <component.h>

namespace nap
{
    
    class ___VARIABLE_className___Instance;
    
    
    class NAPAPI ___VARIABLE_className___ : public ___VARIABLE_baseClass___
    {
        RTTI_ENABLE(___VARIABLE_baseClass___)
        DECLARE_COMPONENT(___VARIABLE_className___, ___VARIABLE_className___Instance)
        
    public:
        ___VARIABLE_className___() : ___VARIABLE_baseClass___() { }
        
    private:
    };

    
    class NAPAPI ___VARIABLE_className___Instance : public ___VARIABLE_baseClass___Instance
    {
        RTTI_ENABLE(___VARIABLE_baseClass___Instance)
    public:
        ___VARIABLE_className___Instance(EntityInstance& entity, Component& resource) : ___VARIABLE_baseClass___Instance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
    private:
    };
        
}
