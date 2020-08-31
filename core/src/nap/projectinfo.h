#pragma once

#include "service.h"
#include <rtti/object.h>
#include "utility/fileutils.h"

namespace nap
{
	class ModuleManager;

	/**
	 * Allows ProjectInfo to find the executable, editor and modules.
	 */
	class NAPAPI PathMapping : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		/**
		 * @return The absolute file path this PathMapping was loaded from
		 */
		const std::string& getFilename() const		{ return mFilename; }

		std::string mProjectExeToRoot;				///< Relative path from project to executable
		std::string mNapkinExeToRoot;				///< Relative path from napkin to executable
		std::vector<std::string> mModulePaths;		///< Relative paths to the module directories

	private:
		std::string mFilename;						
	};


	/**
	 * Contains project related information, including project title, version number,
	 * the names of the modules the project depends on and what data file to load. 
	 * The data file contains the actual app content. The project file describes the project.
	 *
	 * The link to a service configuration file is optional, but when specified it is loaded and applied by Core on startup.
	 * The link to the path mapping file is required, otherwise the system won't be able to resolve all the paths.
	 */
	class NAPAPI ProjectInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
		friend class nap::Core;

	public:
		/**
		 * Controls how paths to modules and other dependencies are resolved.
		 */
		enum class EContext : uint8
		{
			Application = 0,		///< Resolved against running application
			Editor		= 1			///< Resolved against editor
		};

		std::string mTitle;												///< Property: 'Title' project title
		std::string mVersion;											///< Property: 'Version' project version
		std::string mDefaultData;										///< Property: 'Data' relative path to the application content (json) file
		std::string mPathMappingFile;									///< Property: 'PathMapping' relative path to the path mapping file
		std::string mServiceConfigFilename = {};						///< Property: 'ServiceConfig' optional relative path to service configuration file.
		std::vector<std::string> mRequiredModules;						///< Property: 'RequiredModules' names of modules this project depends on

		/**
		 * @return True if this process is running in an editor
		 */
		bool isEditorMode() const;

		/**
		 * @return The absolute file path this ProjectInfo was loaded from
		 */
		std::string getFilename() const;

		/**
		 * Set the filename of this ProjectInfo, only to be called while loading.
		 */
		void setFilename(const std::string& filename);

		/**
		 * @return Absolute path of the project directory
		 */
		std::string getProjectDir() const;

		/**
		 * @return Absolute path to the root of NAP Framework
		 */
		std::string getNAPRootDir() const;

		/**
		 * @return Absolute paths of directories to use when searching for modules
		 */
		std::vector<std::string> getModuleDirectories() const;

		/**
		 * Returns the absolute path to the application data file, which contains application content.
		 * @return Absolute path to the application data file
		 */
		std::string getDataFile() const;

		/**
		 * @return Absolute path of this project's data directory
		 */
		std::string getDataDirectory() const;

		/**
		 * Returns if a service configuration file has been provided.
		 * Service configuration resources deserialized from the configuration file override service defaults.
		 * This means that if no config file is provided, services are initialized using their default configuration resource.
		 * @return if a service configuration file is specified.
		 */
		bool hasServiceConfigFile() const;

		/**
		 * @return The path mapping for this project.
		 */
		const PathMapping& getPathMapping() const;

		/**
		 * Replace all occurrences of specific project variables such as 'ROOT', 'BUILD_CONFIG' etc. with the actual resolved values. 
		 * For example: 
		 * 
		 * ROOT -> path to nap root directory
		 * BUILD_CONFIG -> Clang-Debug-x86_64
		 * BUILD_TYPE -> Release
		 * PROJECT_DIR -> path to project directory
		 *
		 * @param path full path that contains the project variables to replace
		 * @param additionalValues additional variables to resolve
		 * @return if the path is patched
		 */
		void patchPath(std::string& path, const std::unordered_map<std::string, std::string>& additionalValues = {}) const;

		/**
		 * Replace all occurrences of specific project variables such as 'ROOT', 'BUILD_CONFIG' etc. with the actual resolved values.
		 * For example:
		 *
		 * ROOT -> path to nap root directory
		 * BUILD_CONFIG -> Clang-Debug-x86_64
		 * BUILD_TYPE -> Release
		 * PROJECT_DIR -> path to project directory
		 *
		 * @param path full path that contains the project variables to replace
		 * @param additionalValues additional variables to resolve
		 * @return if the path is patched
		 */
		void patchPaths(std::vector<std::string>& paths, const std::unordered_map<std::string, std::string>& additionalValues = {}) const;

	private:
		std::unordered_map<std::string, std::string> getTemplateValues(const std::unordered_map<std::string, std::string>& additionalValues) const;
		std::string mFilename;								///< The filename from which this data was loaded
		std::unique_ptr<PathMapping> mPathMapping;			///< The actual path mapping coming from mPathMappingFile
		EContext mContext = EContext::Application;			///< By default projects are loaded from application context
	};


	/**
	 * Contains module specific information, including the names of the modules this module depends on.
	 */
	class NAPAPI ModuleInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
		friend class nap::ModuleManager;
	public:
		std::vector<std::string> mRequiredModules;		///< Property: 'RequiredModules' names of modules this module depends on
		std::vector<std::string> mLibSearchPaths;		///< Property: 'WindowsDllSearchPaths' additional windows dll search paths

		/**
		 * @return The absolute file path this ModuleInfo was loaded from
		 */
		std::string getFilename() const;

		/**
		 * @return The absolute path to the directory that contains this ModuleInfo file
		 */
		std::string getDirectory() const;

		/**
		 * @return The ProjectInfo instance this ModuleInfo 'belongs' to.
		 */
		const ProjectInfo& getProjectInfo() const;

	private:
		std::string mFilename;				///< The filename from which this data was loaded
		const ProjectInfo* mProjectInfo;	///< The project this module 'belongs' to during the session
	};

} // namespace nap