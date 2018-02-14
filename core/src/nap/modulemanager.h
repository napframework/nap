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
		 * Load all modules in the specified list
		 * @param moduleNames The list of modules to load (temporary?)
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
		
		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}