#pragma once

#include "service.h"
#include <rtti/object.h>

namespace nap
{

	class PathMapping : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		std::string mFilename;
		std::string mProjectExeToRoot;
		std::string mNapkinExeToRoot;
		std::vector<std::string> mModulePaths;
	};

	class ProjectInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		std::string mFilename;					// The filename from which this data was loaded
		std::string mTitle;						// Title of the project
		std::string mVersion;					// Version of this project
		std::string mDefaultData;				// Relative path of the default data (json) file
		std::string mPathMappingFile;			// Points to a file with a path mapping
		std::unique_ptr<PathMapping> mPathMapping; // The actual path mapping coming from mPathMappingFile
		std::vector<std::string> mModuleNames;	// Names of modules this project depends on
		std::vector<nap::ServiceConfiguration*> mServiceConfigurations; // Any service configs in this project


		std::string getFilename() const
		{
			return mFilename;
		}

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
		 * @return Absolute path of this project's data directory
		 */
		std::string dataDirectory() const;
	};

	class ModuleInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		std::string mFilename;					// The filename from which this data was loaded
		std::vector<std::string> mDependencies; // The modules this module depends on
	};
} // namespace nap