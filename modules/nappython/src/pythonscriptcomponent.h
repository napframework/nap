/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
     * The resource part of the PythonScriptComponent. Allows for running a python script inside a NAP application.
     * The python script has to implement a class with the name of the ClassName property.
     * The class should implement:
     * - A constructor that takes the entity instance as an argument
     * - An update method that takes the total elapsed time and the delta time as float arguments.
     * - A destroy method that is called on destruction of the component that does not take any arguments.
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
     * The instance part of the PythonScriptComponent. Allows for running a python script inside a NAP application.
	 * The python script has to implement a class with the name of the ClassName property.
	 * The class should implement:
	 * - A constructor that takes the entity instance as an argument
	 * - An update method that takes the total elapsed time and the delta time as float arguments.
	 * - A destroy method that is called on destruction of the component that does not take any arguments.
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
         * Tries to call a method that returns a value in the python script with the specified number of input arguments.
         * The return value will be stored in returnValue, the identifier is the name of the python method to call.
         * If the call fails the error will be logged in errorState.
		 * @param identifier python method to call.
		 * @param errorState contains the error if the call fails.
		 * @param returnValue the populated return value
		 * @param args variable number of input arguments.
		 * @return if the call succeeded.
         */
        template <typename ReturnType, typename ...Args>
        bool get(const std::string& identifier, utility::ErrorState& errorState, ReturnType& returnValue, Args&&... args);

        /**
         * Tries to call a method in the python script with the specified number of input arguments.
		 * The identifier is the name of the python method to call. 
         * If the call fails the error will be logged in errorState.
		 * @param identifier python method to call
		 * @param errorState contains the error if the call fails
		 * @param args variable number of input arguments.
		 * @return if the call succeeded
         */
        template <typename ...Args>
        bool call(const std::string& identifier, utility::ErrorState& errorState, Args&&... args);

    private:
        PythonScriptComponent* mResource = nullptr;
        pybind11::object mInstance;
        bool mInitialized = false;
    };


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

    template <typename ReturnType, typename ...Args>
    bool PythonScriptComponentInstance::get(const std::string& identifier, utility::ErrorState& errorState, ReturnType& returnValue, Args&&... args)
    {
        try
        {
            returnValue = mInstance.attr(identifier.c_str())(args...).template cast<ReturnType>();
        }
        catch (const pybind11::error_already_set& err)
        {
            errorState.fail("Runtime python error while executing %s: %s", mResource->mPythonScript->mPath.c_str(), err.what());
            return false;
        }
        return true;
    }


    template <typename ...Args>
    bool PythonScriptComponentInstance::call(const std::string& identifier, utility::ErrorState& errorState, Args&&... args)
    {
        try
        {
            mInstance.attr(identifier.c_str())(std::forward<Args>(args)...);
        }
        catch (const pybind11::error_already_set& err)
        {
            errorState.fail("Runtime python error while executing %s: %s", mResource->mPythonScript->mPath.c_str(), err.what());
            return false;
        }
        return true;
    }
}
