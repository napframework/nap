#pragma once

#include <utility/dllexport.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>
#include "nap/service.h"

namespace nap
{
	/**
	 * Python script services. Manages the python module and environment.
	 */
	class NAPAPI PythonScriptService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		PythonScriptService(ServiceConfiguration* configuration);

		/**
		 * Called by core. Registers all object creation methods for python specific NAP objects.
		 * @param factory the factory to register the creation functions with.
		 */
        void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Tries to load a python module into the NAP environment.
		 * @param modulePath path to the python module to load
		 * @param module the loaded module
		 * @param errorState contains the error when loading failed.
		 * @return if loading the module succeeded or not
		 */
		bool TryLoad(const std::string& modulePath, pybind11::module& module, utility::ErrorState& errorState);
        
	private:
		using ModuleMap = std::unordered_map<std::string, pybind11::module>;
		using SystemPathSet = std::unordered_set<std::string>;

		pybind11::scoped_interpreter	mInterpreter;	// Note: must be first member to ensure it's destroyed last. Otherwise module destruction will crash.
		ModuleMap						mLoadedModules;
		SystemPathSet					mSystemPaths;
		
	};
}
