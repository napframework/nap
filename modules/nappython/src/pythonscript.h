#pragma once

// Pybind includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Nap includes
#include <nap/resource.h>
#include <nap/logger.h>
#include <rtti/factory.h>
#include <utility/errorstate.h>

namespace nap {

    // Forward declarations
    class PythonScriptService;

    /**
     * A python script loaded from a text file into a pybind11 module.
     * Functions within the script can be called using the @call() method and handles to functions and classes can be acquired using @get()
     */
    class NAPAPI PythonScript : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        PythonScript(PythonScriptService& service);

        std::string mPath; ///< property: 'Path' Path to the python script.

        bool init(utility::ErrorState& errorState) override;

        /**
         * Tries to call a method that returns a value with name @identifier in the python script with the specified arguments @args.
         * The return value will be stored in @returnValue.
         * If the call fails the error will be logged in errorState.
         */
        template <typename ReturnType, typename ...Args>
        bool get(const std::string& identifier, utility::ErrorState& errorState, ReturnType& returnValue, Args&&... args);

        /**
         * Tries to call a method with name @identifier in the python script with the specified arguments @args.
         * If the call fails the error will be logged in errorState.
         */
        template <typename ...Args>
        bool call	(const std::string& identifier, utility::ErrorState& errorState, Args&&... args);

        /**
         * Requests a symbol (in most cases a class or function) from the script and returns its C++ representation.
         */
        pybind11::object get(const std::string& symbol);

    private:
        pybind11::module mModule;
        PythonScriptService* mService = nullptr;
    };


    template <typename ReturnType, typename ...Args>
    bool PythonScript::get(const std::string& identifier, utility::ErrorState& errorState, ReturnType& returnValue, Args&&... args)
    {
        try
        {
            returnValue = mModule.attr(identifier.c_str())(std::forward<Args>(args)...).template cast<ReturnType>();
        }
        catch (const pybind11::error_already_set& err)
        {
            errorState.fail("Runtime python error while executing %s: %s", mPath.c_str(), err.what());
            return false;
        }
        return true;
    }


    template <typename ...Args>
    bool PythonScript::call(const std::string& identifier, utility::ErrorState& errorState, Args&&... args)
    {
        try
        {
            mModule.attr(identifier.c_str())(std::forward<Args>(args)...);
        }
        catch (const pybind11::error_already_set& err)
        {
            errorState.fail("Runtime python error while executing %s: %s", mPath.c_str(), err.what());
            return false;
        }
        return true;
    }


    // Object creator used for constructing the the PythonScript
    using PythonScriptObjectCreator = rtti::ObjectCreator<PythonScript, PythonScriptService>;

}
