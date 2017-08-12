#include <rtti/pythonmodule.h>
#include "pythonscriptservice.h"
#include "nap/fileutils.h"

RTTI_BEGIN_CLASS(nap::PythonScriptService)
RTTI_END_CLASS

namespace nap
{
	bool PythonScriptService::TryLoad(const std::string& modulePath, pybind11::module& module, utility::ErrorState& errorState)
	{
		try 
		{
			std::string script_directory = getAbsolutePath(getFileDir(modulePath));
			if (mSystemPaths.find(toComparableFilename(script_directory)) == mSystemPaths.end())
			{
				PyObject* sysPath = PySys_GetObject((char*)"path");
				PyList_Append(sysPath, Py_BuildValue("s", script_directory.c_str()));

				mSystemPaths.insert(toComparableFilename(script_directory));
			}

			ModuleMap::iterator existing_module = mLoadedModules.find(toComparableFilename(modulePath));
			if (existing_module != mLoadedModules.end())
			{
				PyObject* reloaded_module = PyImport_ReloadModule(existing_module->second.ptr());
				if (reloaded_module == nullptr)
					throw pybind11::error_already_set();

				module = existing_module->second;
			}
			else
			{
				pybind11::module new_module = pybind11::module::import(getFileNameWithoutExtension(modulePath).c_str());

				auto inserted = mLoadedModules.emplace(std::make_pair(toComparableFilename(modulePath), new_module));
				module = inserted.first->second;
			}
		}
		catch (const pybind11::error_already_set& err)
		{
			errorState.fail(err.what());
			return false;
		}

		return true;
	}
}