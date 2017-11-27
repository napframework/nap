#pragma once

#include <utility/dllexport.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "nap/service.h"

namespace nap
{
	class NAPAPI PythonScriptService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		bool TryLoad(const std::string& modulePath, pybind11::module& module, utility::ErrorState& errorState);

	private:
		using ModuleMap = std::unordered_map<std::string, pybind11::module>;
		using SystemPathSet = std::unordered_set<std::string>;

		pybind11::scoped_interpreter	mInterpreter;	// Note: must be first member to ensure it's destroyed last. Otherwise module destruction will crash.
		ModuleMap						mLoadedModules;
		SystemPathSet					mSystemPaths;
		
	};
}
