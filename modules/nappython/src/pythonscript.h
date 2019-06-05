#pragma once

// Pybind includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Nap includes
#include <nap/resource.h>
#include <nap/logger.h>
#include <rtti/factory.h>

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
         * Tries to call a function with name @identifier in the python script with the specified arguments @args.
         * The return type will be inferred, if the function has no return type use call<void>().
         * If the call fails the error will be logged.
         */
        template <typename ReturnType, typename ...Args>
        ReturnType call(const std::string& identifier, Args... args);
        
        // Specialization for void return type
        template <typename ...Args>
        void call(const std::string& identifier, Args... args);
        
        /**
         * Requests a symbol (in most cases a class or function) from the script and returns its C++ representation.
         */
        pybind11::object get(const std::string& symbol);
        
    private:
        pybind11::module mModule;
        PythonScriptService* mService = nullptr;
    };
    
    
    template <typename ReturnType, typename ...Args>
    ReturnType PythonScript::call(const std::string& identifier, Args... args)
    {
        try
        {
            return mModule.attr(identifier.c_str())(args...).template cast<ReturnType>();
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::error("Runtime python error while executing %s: %s", mPath.c_str(), err.what());
        }
    }

    
    template <typename ...Args>
    void PythonScript::call(const std::string& identifier, Args... args)
    {
        try
        {
            mModule.attr(identifier.c_str())(args...);
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::error("Runtime python error while executing %s: %s", mPath.c_str(), err.what());
        }
    }
    
    
    // Object creator used for constructing the the PythonScript
    using PythonScriptObjectCreator = rtti::ObjectCreator<PythonScript, PythonScriptService>;
    
}
