#pragma once

// STD includes
#include <string>
#include <vector>

// Core includes
#include "utility/module.h"
#include "utility/errorstate.h"
#include "rtti/typeinfo.h"
#include "projectinfo.h"

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
		/**
		* Data for a loaded module
		*/
		struct Module
		{
			std::string						mName;									// The canonical name of the module
			ModuleDescriptor*				mDescriptor;							// The descriptor that belongs to the module
			std::unique_ptr<ModuleInfo>		mInfo;									// Data that was loaded from the module json
			void*							mHandle;								// Handle to native module
			rtti::TypeInfo					mService = rtti::TypeInfo::empty();		// Service associated with the module
		};


		// Constructor
		ModuleManager(Core& core);

		// Destructor
		~ModuleManager();

		/**
		 * Load all modules specified in the project info
		 * @param projectInfo The descriptor providing the module dependencies
		 * @param error Any errors will be stored here
		 * @return True on success, false otherwise
		 */
		bool loadModules(const ProjectInfo& projectInfo, utility::ErrorState& error);

		/**
		 * @return All currently loaded modules
		 */
		std::vector<nap::ModuleManager::Module*> getModules() const;

	private:
		bool loadModule_(const ProjectInfo& projectinfo, const std::string& moduleFile, utility::ErrorState& err);


		/**
		 * Retrieve a loaded module by its name as defined in its descriptor file
		 * @param moduleName The name of the module to find
		 * @return The object providing access to the Module
		 */
		const Module* getLoadedModule(const std::string& moduleName);

		/**
		 * Given a ProjectInfo descriptor and a module name, retrieve the filename of the module and its json descriptor
		 * @param projectInfo The current project info descriptor we're using for this session
		 * @param moduleName The name of the module to find
		 * @param moduleFile The absolute path to resulting module
		 * @param moduleJson The absolute path to the resulting module json descriptor
		 * @return True if the operation was successful, false otherwise
		 */
		bool findModuleFiles(const ProjectInfo& projectInfo, const std::string& moduleName,
							 std::string& moduleFile, std::string& moduleJson);


		std::vector<std::unique_ptr<Module>> 	mModules;	// The loaded modules
		Core& 								 	mCore;		// Core

	};
}
