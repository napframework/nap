#pragma once

#include "utility/module.h"
#include <string>
#include <vector>

namespace nap
{
	/**
	 * Responsible for dynamically loading additional nap modules
	 */
	class NAPAPI ModuleManager
	{
	public:
		ModuleManager();
		~ModuleManager();

		/**
		 * Load all modules in the specified directory
		 * @param directory The directory to load modules from (relative to current working directory). Defaults to current working directory.
		 */
		void loadModules(const std::string& directory = ".");

	private:
		/**
		 * Data for a loaded module
		 */
		struct Module
		{
			ModuleDescriptor*	mDescriptor;	// The descriptor that belongs to the module
			void*				mHandle;		// Handle to native module
		};

		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}
