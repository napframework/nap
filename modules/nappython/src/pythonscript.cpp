/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pythonscript.h"
#include <pythonscriptservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PythonScript)
    RTTI_PROPERTY_FILELINK("Path", &nap::PythonScript::mPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Python)
RTTI_END_CLASS

namespace nap
{
    
    PythonScript::PythonScript(PythonScriptService& service) : Resource(), mService(&service)
    {
    }

    
    bool PythonScript::init(utility::ErrorState& errorState)
    {
        // Load the script
        if (!errorState.check(mService->TryLoad(mPath, mModule, errorState), "Failed to load %s", mPath.c_str()))
            return false;
        
        return true;
    }
    
    
    pybind11::object PythonScript::get(const std::string& symbol)
    {
        try
        {
            return mModule.attr(symbol.c_str());
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::error("Runtime python error while getting symbol %s in %s: %s", symbol.c_str(), mPath.c_str(), err.what());
            return pybind11::object(); // return empty object
        }
    }

    
}
