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
		bool buildModuleSearchDirectories(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories);

		/**
		 * Attempt to parse the build type from a folder name in the build system
		 * TODO: Being used temporarily to detect our NAP source / NAP packaged / packaged project environment
		 *       until we're packaging information during our build to help us determine tbis
		 *
		 * A valid full build configuration string in a folder name will be of our format specified in CMake:
		 * macOS: COMPILER_ID-ARCH-BUILD_TYPE, eg. Clang-x86_64-Debug
		 * Linux: COMPILER_ID-BUILD_TYPE-ARCH, eg. GNU-ReleaseWithDebInfo-x86_64
		 *
		 * @param folderName The folder name to parse
		 * @param outBuildType The output build type
		 * @return Whether a build type was parsed from the folder name
		 */
		bool getBuildTypeFromFolder(std::string& folderName, std::string& outBuildType);

		using ModuleList = std::vector<Module>;
		ModuleList mModules;	// The loaded modules
	};
}
