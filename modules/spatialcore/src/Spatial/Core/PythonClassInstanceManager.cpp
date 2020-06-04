//
//  PythonClass.cpp
//  Project
//
//  Created by Casimir Geelhoed on 23/08/2019.
//
//

#include "PythonClassInstanceManager.h"

#include <Spatial/Core/ParameterManager.h>


namespace nap
{

    namespace spatial
    {

        bool PythonClassInstanceManager::init(nap::PythonScript* pythonScript, pybind11::object* pythonClass, nap::EntityInstance* entity, std::string instanceName, ParameterManager* parameterManager)
        {
            // copy pointers
            mPythonClass = pythonClass;
            mPythonScript = pythonScript;
            mParameterManager = parameterManager;

            // instantiate python class
            try
            {
                mInstance = pythonClass->operator()(entity, instanceName, parameterManager);
            }
            catch (const pybind11::error_already_set& err)
            {
                nap::Logger::error("Runtime python error while instantiating pythonscript: %s.", err.what());
                return false;
            }
            return true;
        }

    }

}
