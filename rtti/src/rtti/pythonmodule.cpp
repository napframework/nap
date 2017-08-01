#pragma once

#include "pythonmodule.h"
#include "utility/stringutils.h"
#include <unordered_map>

#include "windows.h"

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
			std::vector<std::string> base_types;
			itemToRegister->second.mGetBaseTypesFunction(base_types);

			for (const std::string& dependent_type : base_types)
			{
				RegistrationMap::iterator dependent_item = mRegistrationMap.find(dependent_type);
				if (dependent_item != mRegistrationMap.end())
				{
					registerTypeRecursive(module, dependent_item);
					mRegistrationMap.erase(dependent_item);
				}
			}

			itemToRegister->second.mRegistrationFunction(module);
			OutputDebugString(utility::stringFormat("%s\n", itemToRegister->first.c_str()).c_str());
		}

		void PythonModule::invoke(pybind11::module& module)
		{
			for (RegistrationMap::iterator pos = mRegistrationMap.begin(); pos != mRegistrationMap.end();)
			{
				registerTypeRecursive(module, pos);
				pos = mRegistrationMap.erase(pos);
			}				

			for (auto& func : mImportCallbacks)
				func(module);
		}
	}
}