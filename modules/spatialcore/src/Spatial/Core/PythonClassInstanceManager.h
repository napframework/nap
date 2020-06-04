#pragma once

// Nap includes
#include <entity.h>
#include <nap/resource.h>
#include <nap/logger.h>
#include <pythonscript.h>

namespace nap
{

    // Forward declarations
    class PythonScript;
    class ParameterManager;

    namespace spatial
    {
        
        // __________________________________________________________________________________________
        /**
         * The pythonscript-related functionality of PythonScriptComponent in a separate class,
         * so we can use its functionality without having to create an extra PythonScriptComponent every time.
         * IDEA: I would suggest that this class gets moved next to the pythonscript.h resource file in NAP and that
         * PythonScriptComponent will also use this class to interact with the PythonScript resource.
         * Note Casi: added ParameterManager* to the constructor after Parameter refactor. So the above maybe doesn't apply anymore.
         */
        class NAPAPI PythonClassInstanceManager {
            
        public:
            
            bool init(nap::PythonScript* pythonScript, pybind11::object* pythonClass, nap::EntityInstance* entity, std::string instanceName, ParameterManager* parameterManager);
            
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
            nap::PythonScript* mPythonScript = nullptr;
            pybind11::object* mPythonClass = nullptr;
            ParameterManager* mParameterManager = nullptr;
            pybind11::object mInstance;
            
        };
        
        template <typename ReturnType, typename ...Args>
        ReturnType PythonClassInstanceManager::call(const std::string& identifier, Args... args)
        {
            try
            {
                return mInstance.attr(identifier.c_str())(args...).template cast<ReturnType>();
            }
            catch (const pybind11::error_already_set& err)
            {
                nap::Logger::error("Runtime python error while executing %s: %s", mPythonScript->mPath.c_str(), err.what());
                return ReturnType();
            }
        }
        
        
        template <typename ...Args>
        void PythonClassInstanceManager::call(const std::string& identifier, Args... args)
        {
            try
            {
                mInstance.attr(identifier.c_str())(args...);
            }
            catch (const pybind11::error_already_set& err)
            {
                nap::Logger::error("Runtime python error while executing %s: %s", mPythonScript->mPath.c_str(), err.what());
            }
        }
        
        
    }
}
