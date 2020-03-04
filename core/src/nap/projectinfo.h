#pragma once
#include <rtti/typeinfo.h>

namespace nap
{
	class ProjectInfo
	{
		RTTI_ENABLE()
	public:
		std::string mFilename; // The filename from which this data was loaded
		std::string mTitle; // Title of the project
		std::string mVersion; // Version of this project
		std::string mDefaultData; // Relative path of the default data (json) file
		std::vector<std::string> mModuleNames; // Names of modules this project depends on
		std::vector<std::string> mModuleDirs; // Relative or absolute directory paths to search for modules

		std::string getFilename() const { return mFilename; }

		/**
		 * @return Absolute path of the project directory
		 */
		std::string getDirectory() const;

		/**
		 * @return Absolute paths of directories to use when searching for modules
		 */
		std::vector<std::string> getModuleDirectories() const;

		/**
		 * @return Absolute path of the default data file
		 */
		std::string getDefaultDataFile() const;

		/**
		 * Load the given filename and apply its data onto this object
		 */
		bool load(const std::string& filename, utility::ErrorState& err);

		/**
		 * @return Absolute path of this project's data directory
		 */
		std::string dataDirectory() const;
	};

	class ModuleInfo
	{
		RTTI_ENABLE()
	public:
		std::string mFilename; // The filename from which this data was loaded
		std::vector<std::string> mDependencies; // The modules this module depends on

		/**
		 * Load the given filename and apply its data onto this object
		 */
		bool load(const std::string& filename, utility::ErrorState& err);
	};
}