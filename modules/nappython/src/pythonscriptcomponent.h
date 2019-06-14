#pragma once

// Pybind includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Nap includes
#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include <componentptr.h>
#include <nap/logger.h>
#include <instanceproperty.h>
#include <rtti/object.h>
#include <nap/resourceptr.h>
#include <entityptr.h>
#include <pythonscriptservice.h>
#include <pythonscript.h>


namespace nap
{
    
    // Forward declarations
    class PythonScriptComponent;
    class PythonScriptComponentInstance;
    

    /**
     * The resource class for the PythonScriptComponent.
     * The python script has to implement a class with the name of the ClassName property.
     * The class should implement:
     * - A constructor that takes the entity instance as an argument
     * - An update method that takes the total ellapsed time and the delta time as float arguments.
     * - A destroy method that is called on destruction of the component that doesn not take any arguments.
     */
    class NAPAPI PythonScriptComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(PythonScriptComponent, PythonScriptComponentInstance)
        
        friend class PythonScriptComponentInstance;
        
    public:
        bool init(utility::ErrorState& errorState) override;
        
        ResourcePtr<PythonScript> mPythonScript = nullptr;  ///< property: 'PythonScript' Pointer to a python script resource that manages the script that contains the python class for this component.
        std::string mClassName;                             ///< property: 'Class' The name of the class defined in the python script
        std::vector<std::string> mDependencies;             ///< property: 'Dependencies' list of component types that need to be among this scripts siblings and that will be initialized before this component.
        
        // Inherited from Component
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
        pybind11::module mModule;
        pybind11::object mPythonClass;
    };
    
    
    /**
     * Instance of a @PythonScriptComponent.
     */
    class NAPAPI PythonScriptComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        PythonScriptComponentInstance(EntityInstance& entity, Component& resource);
        ~PythonScriptComponentInstance();
        
        // Inherited from ComponentInstance
        virtual void update(double deltaTime) override;
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * Tries to call a method with name @identifier in the python script with the specified arguments @args.
         * The return type will be inferred, if the function has no return type use call<void>().
         * If the call fails the error will be logged.
         */
        template <typename ReturnType, typename ...Args>
        ReturnType call(const std::string& identifier, Args... args);
        
        // Specialization for void return type
        template <typename ...Args>
        void call(const std::string& identifier, Args... args);
        
    private:
        PythonScriptComponent* mResource = nullptr;
        pybind11::object mInstance;
        bool mInitialized = false;
    };
    
    
    template <typename ReturnType, typename ...Args>
    ReturnType PythonScriptComponentInstance::call(const std::string& identifier, Args... args)
    {
        try
        {
            return mInstance.attr(identifier.c_str())(args...).template cast<ReturnType>();
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::error("Runtime python error while executing %s: %s", mResource->mPythonScript->mPath.c_str(), err.what());
			return ReturnType();
        }
    }
    

    template <typename ...Args>
    void PythonScriptComponentInstance::call(const std::string& identifier, Args... args)
    {
        try
        {
            mInstance.attr(identifier.c_str())(args...);
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::error("Runtime python error while executing %s: %s", mResource->mPythonScript->mPath.c_str(), err.what());
        }
    }
    
    
}
