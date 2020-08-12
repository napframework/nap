#pragma once

#include "service.h"
#include <rtti/object.h>
#include "utility/fileutils.h"

namespace nap
{
	class ModuleManager;

	class NAPAPI PathMapping : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		std::string mProjectExeToRoot;
		std::string mNapkinExeToRoot;
		std::vector<std::string> mModulePaths;

		/**
		 * @return The absolute file path this PathMapping was loaded from
		 */
		std::string getFilename() const
		{
			return mFilename;
		}

	private:
		std::string mFilename;
	};


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

		std::string mTitle;												// Title of the project
		std::string mVersion;											// Version of this project
		std::string mDefaultData;										// Relative path of the default data (json) file
		std::string mPathMappingFile;									// Points to a file with a path mapping
		std::string mServiceConfigFilename = {};						// Points to a file with service configurations
		std::vector<std::string> mRequiredModules;						// Names of modules this project depends on

		/**
		 * @return True if this process is running in an editor
		 */
		bool isEditorMode() const;

		/**
		 * @return The absolute file path this ProjectInfo was loaded from
		 */
		std::string getFilename() const;

		/**
		 * Set the filename of this projectinfo, only to be called while loading.
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
		 * @return Absolute path of the default data file
		 */
		std::string getDefaultDataFile() const;

		/**
		 * @return Absolute path of this project's data directory
		 */
		std::string dataDirectory() const;


		bool hasServiceConfigFile() const;
		/**
		 * @return The path mapping for this project
		 */
		const PathMapping& getPathMapping() const;

		bool patchPath(std::string& path, const std::unordered_map<std::string, std::string>& additionalValues = {}) const;

		bool patchPaths(std::vector<std::string>& paths, const std::unordered_map<std::string, std::string>& additionalValues = {}) const;

	private:
		std::unordered_map<std::string, std::string> getTemplateValues(const std::unordered_map<std::string, std::string>& additionalValues) const;

		std::unordered_map<rtti::TypeInfo, std::unique_ptr<ServiceConfiguration>> mServiceConfigs;
		std::string mFilename;								// The filename from which this data was loaded
		std::unique_ptr<PathMapping> mPathMapping;			// The actual path mapping coming from mPathMappingFile
		EContext mContext = EContext::Application;			// By default projects are loaded from application context
	};


	class NAPAPI ModuleInfo : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
		friend class nap::ModuleManager;

	public:
		std::vector<std::string> mRequiredModules; // The modules this module depends on
		std::vector<std::string> mLibSearchPaths;

		/**
		 * @return The absolute file path this ModuleInfo was loaded from
		 */
		std::string getFilename() const
		{
			return mFilename;
		}

		/**
		 * @return The ProjectInfo instance this ModuleInfo 'belongs' to.
		 */
		const ProjectInfo& getProjectInfo() const;

	private:
		std::string mFilename;			 // The filename from which this data was loaded
		const ProjectInfo* mProjectInfo; // The project this module 'belongs' to during the session
	};


} // namespace nap