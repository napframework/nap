#pragma once

// STD includes
#include <string>
#include <vector>

// Core includes
#include "utility/module.h"
#include "utility/errorstate.h"
#include "rtti/typeinfo.h"

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
		 * Load all modules in the specified list.  Two modes of operationg are provided:
		 * - If the module list is populated only modules in that list will be loaded and if any cannot be loaded
		 *   initialization will fail
		 * - If no module list is provided all encountered modules will be loaded
		 * @param moduleNames The list of modules to load
		 * @return Whether we were successfully able load our requested modules
		 */
		bool loadModules(std::vector<std::string>& moduleNames, utility::ErrorState& error);

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
		 * @return whether we were successfully able to determine our configuration and build our search paths
		 */
		bool buildModuleSearchDirectories(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories, utility::ErrorState& errorState);


		/**
		 * Build directories to search in for specified modules for a non packaged project running against released NAP
		 * @param moduleNames The names of the modules in use in the project
		 * @param outSearchDirectories The directories to search for the provided modules
		 */
		void buildPackagedNapProjectModulePaths(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories);

		/**
		 * Build directories to search in for all modules for non project context (eg. napkin) running against released NAP
		 * @param outSearchDirectories The directories to search for the provided modules
		 */
		void buildPackagedNapNonProjectModulePaths(std::vector<std::string>& outSearchDirectories);

		/**
		 * Check whether the specified folder name contains a build configuration.
		 *
		 * A valid full build configuration string in a folder name will be of our format specified in CMake:
		 * macOS: COMPILER_ID-ARCH-BUILD_TYPE, eg. Clang-x86_64-Debug
		 * Linux: COMPILER_ID-BUILD_TYPE-ARCH, eg. GNU-Release-x86_64
		 *
		 * The folder name is checked for sufficient parts and a matching build type.
		 *
		 * @param folderName The folder name to parse
		 * @return Whether the folder name appears to contain a build configuration
		 */
		bool folderNameContainsBuildConfiguration(std::string& folderName);
		
		/**
		 * Load a list of NAP module dependencies from the specifed module JSON file
		 * @param jsonFile The module.json file to load from
		 * @param outDependencies The loaded list of dependencies
		 * @param error The error message when loading fails
		 * @return If loading failed or succeeded
		 */
		bool loadModuleDependenciesFromJSON(std::string& jsonFile, std::vector<std::string>& outDependencies, utility::ErrorState& error);
		
		/**
		 * For the specified top level NAP modules load a list of all NAP module dependencies
		 * @param topLevelProjectModules The NAP modules that our project requires, to initiate our search from
		 * @param outDependencies The loaded list of dependencies
		 * @param error The error message when the process fails
		 * @return If the process failed or succeeded
		 */
		bool fetchProjectModuleDependencies(std::vector<std::string>& topLevelProjectModules, std::vector<std::string>& outDependencies, utility::ErrorState& errorState);

		/**
		 * For the specified NAP modules load a list of their immediate NAP module dependencies that we haven't already found
		 * @param searchModules The set of modules to load immediate dependencies for
		 * @param previouslyFoundModules A list of NAP module dependencies which we have already found. If any of these modules are encountered they won't be appended to outDependencies.
		 * @param outDependencies The loaded list of dependencies
		 * @param error The error message when the process fails
		 * @return If the process failed or succeeded
		 */
		bool fetchImmediateModuleDependencies(std::vector<std::string>& searchModules, std::vector<std::string>& previouslyFoundModules, std::vector<std::string>& outDependencies, utility::ErrorState& errorState);
		
		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}
