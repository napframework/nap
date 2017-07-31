#pragma once

#include "pythonmodule.h"
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

		void PythonModule::invoke(pybind11::module& module)
		{
			for (auto& func : mImportCallbacks)
				func(module);
		}
	}
}