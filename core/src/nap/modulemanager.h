#pragma once

#include "utility/module.h"
#include "rtti/typeinfo.h"
#include <string>
#include <vector>

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Responsible for dynamically loading additional nap modules
	 */
	class NAPAPI ModuleManager final
	{
		friend class Core;
	public:
		// Constructor
		ModuleManager();

		// Destructor
		~ModuleManager();

		/**
		 * Load all modules in the specified list
		 * @param moduleNames The list of modules to load (temporary?)
		 */
		void loadModules(std::vector<std::string>& moduleNames);
		

	private:
		/**
		* Data for a loaded module
		*/
		struct Module
		{
			ModuleDescriptor*	mDescriptor;							// The descriptor that belongs to the module
			void*				mHandle;								// Handle to native module
			rtti::TypeInfo		mService = rtti::TypeInfo::empty();		// Service associated with the module
		};

		/**
		 * Build directories to search in for specified modules
		 * @param moduleNames The names of the modules in use in the project
		 * @param outSearchDirectories The directories to search for the provided modules
		 */
		void buildModuleSearchDirectories(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories);

		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}
