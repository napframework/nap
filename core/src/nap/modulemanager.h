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
		 * Load all modules in the specified directories
		 * @param directories The directories to load modules from (relative to current working directory)
		 */
		void loadModules(const std::vector<std::string>& directories);
		

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

		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}
