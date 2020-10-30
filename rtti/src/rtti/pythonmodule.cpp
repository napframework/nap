/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifdef NAP_ENABLE_PYTHON
#include "pythonmodule.h"
#include <utility/stringutils.h>
#include <unordered_map>

namespace nap
{
	namespace rtti
	{
		PythonModule& PythonModule::get(const char* moduleName)
		{
			static std::unordered_map<std::string, PythonModule> pythonModules;
			return pythonModules[std::string(moduleName)];
		}


		void PythonModule::registerImportCallback(RegistrationFunction function)
		{
			mImportCallbacks.push_back(function);
		}


		void PythonModule::registerTypeImportCallback(const std::string& name, GetBaseTypesFunction getBaseTypesFunction, RegistrationFunction registrationFunction)
		{
			RegistrationItem item;
			item.mGetBaseTypesFunction = getBaseTypesFunction;
			item.mRegistrationFunction = registrationFunction;
			mRegistrationMap.emplace(std::make_pair(name, std::move(item)));
		}


		void PythonModule::registerTypeRecursive(pybind11::module& module, RegistrationMap::iterator itemToRegister)
		{
			// Important: we call the getBaseTypes function here (as late as possible), so that we are sure
			// that all types are registered into RTTI, otherwise we can get guessed names from RTTI that do 
			// not necessarily match
			std::vector<std::string> base_types;
			itemToRegister->second.mGetBaseTypesFunction(base_types);

			// Go over base types
			for (const std::string& base_type : base_types)
			{
				// If not already registered, recurse it that type's base types
				RegistrationMap::iterator dependent_item = mRegistrationMap.find(base_type);
				if (dependent_item != mRegistrationMap.end())
				{
					registerTypeRecursive(module, dependent_item);
					// We've now registered this type, make sure we remote it so that it does not get registered multiple times
					mRegistrationMap.erase(dependent_item);
				}
			}

			// All bases are registered, register ourself
			itemToRegister->second.mRegistrationFunction(module);
		}


		void PythonModule::invoke(pybind11::module& module)
		{
			// Go through all registered types
			for (RegistrationMap::iterator pos = mRegistrationMap.begin(); pos != mRegistrationMap.end();)
			{
				// Recurse into base types
				registerTypeRecursive(module, pos);

				// When we've registered a type, erase it from the list so that it doesn't get registered multiple times.
				pos = mRegistrationMap.erase(pos);
			}				

			// Invoke all regular import callbacks
			for (auto& func : mImportCallbacks)
				func(module);
		}
	}
}
#endif // NAP_ENABLE_PYTHON