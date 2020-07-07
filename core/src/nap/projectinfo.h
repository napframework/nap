#pragma once

#include "service.h"
#include <rtti/object.h>
#include "utility/fileutils.h"

namespace nap
{
	class ModuleManager;

	class PathMapping : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		std::string mProjectExeToRoot;
		std::string mNapkinExeToRoot;
		std::vector<std::string> mModulePaths;

		/**
		 * @return The absolute file path this PathMapping was loaded from
		 */
		std::string getFilename() const { return mFilename; }

	private:
		std::string mFilename;
	};

	class ProjectInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
		friend class nap::Core;
	public:
		std::string mTitle;						// Title of the project
		std::string mVersion;					// Version of this project
		std::string mDefaultData;				// Relative path of the default data (json) file
		std::string mPathMappingFile;			// Points to a file with a path mapping
		std::vector<std::string> mRequiredModules;	// Names of modules this project depends on
		std::vector<nap::ServiceConfiguration*> mServiceConfigurations; // Any service configs in this project

		/**
		 * @return True if this process is running in an editor
		 */
		bool isEditorMode() const { return mEditorMode; }

		/**
		 * WARNING: Only to be used from the editor.
		 */
		void setEditorMode() { mEditorMode = true; }

		/**
		 * @return The absolute file path this ProjectInfo was loaded from
		 */
		std::string getFilename() const { return mFilename; }

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

		/**
		 * @return The path mapping for this project
		 */
		const PathMapping& getPathMapping() const;

	private:
		std::string mFilename;					// The filename from which this data was loaded
		std::unique_ptr<PathMapping> mPathMapping; // The actual path mapping coming from mPathMappingFile
		bool mEditorMode = false;
	};

	class ModuleInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
		friend class nap::ModuleManager;
	public:
		std::vector<std::string> mRequiredModules; 	// The modules this module depends on
		std::vector<std::string> mLibSearchPaths;

		/**
		 * @return The absolute file path this ModuleInfo was loaded from
		 */
		std::string getFilename() const { return mFilename; }

		/**
		 * @return The ProjectInfo instance this ModuleInfo 'belongs' to.
		 */
		const ProjectInfo& getProjectInfo() const;

	private:
		std::string mFilename;						// The filename from which this data was loaded
		const ProjectInfo* mProjectInfo;			// The project this module 'belongs' to during the session
	};
} // namespace nap