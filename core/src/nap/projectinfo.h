#pragma once

#include "service.h"
#include <rtti/object.h>
#include "utility/fileutils.h"

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
		std::vector<std::string> mRequiredModules;	// Names of modules this project depends on
		std::vector<nap::ServiceConfiguration*> mServiceConfigurations; // Any service configs in this project

		// not in RTTI
		std::unique_ptr<PathMapping> mPathMapping; // The actual path mapping coming from mPathMappingFile
		bool mEditorMode = false;


		std::string getFilename() const
		{
			return mFilename;
		}

		/**
		 * @return Absolute path of the project directory
		 */
		std::string getProjectDir() const {
			assert(!mFilename.empty());
			auto f = nap::utility::getAbsolutePath(mFilename);
			return nap::utility::getFileDir(f);
		}

		/**
		 * @return Absolute path to the root of NAP Framework
		 */
		std::string getNAPRootDir() const;

		/**
		 * @return Absolute paths of directories to use when searching for modules
		 */
		std::vector<std::string> getModuleDirectories() const;

		/**
		 * @return Absolute path of the default data file
		 */
		std::string getDefaultDataFile() const {if (mDefaultData.empty())
				return {};
			return utility::joinPath({getProjectDir(), mDefaultData});}

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
		std::vector<std::string> mRequiredModules; // The modules this module depends on
		std::vector<std::string> mLibSearchPaths;
	};
} // namespace nap