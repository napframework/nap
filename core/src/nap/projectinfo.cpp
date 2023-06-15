/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "projectinfo.h"
#include "core.h"
#include "logger.h"

// External Includes
#include <utility/fileutils.h>
#include <rtti/factory.h>
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <nap/assert.h>

RTTI_BEGIN_CLASS(nap::PathMapping)
	RTTI_PROPERTY("ProjectExeToRoot",		&nap::PathMapping::mProjectExeToRoot,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NapkinExeToRoot",		&nap::PathMapping::mNapkinExeToRoot,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ModulePaths",			&nap::PathMapping::mModulePaths,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputPath",				&nap::PathMapping::mOutputPath,				nap::rtti::EPropertyMetaData::Required)	
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ProjectInfo)
	RTTI_PROPERTY("Title",					&nap::ProjectInfo::mTitle,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Version",				&nap::ProjectInfo::mVersion,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Data",					&nap::ProjectInfo::mDefaultData,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathMapping",			&nap::ProjectInfo::mPathMappingFile,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ServiceConfig",			&nap::ProjectInfo::mServiceConfigFilename,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RequiredModules",		&nap::ProjectInfo::mRequiredModules,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ModuleInfo)
	RTTI_PROPERTY("RequiredModules",		&nap::ModuleInfo::mRequiredModules,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WindowsDllSearchPaths",	&nap::ModuleInfo::mLibSearchPaths,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DataSearchPaths",		&nap::ModuleInfo::mDataSearchPaths,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	std::string ProjectInfo::getNAPRootDir() const
	{
		assert(!mRoot.empty());
		return mRoot;
	}


	std::string ProjectInfo::getBuildDir() const
	{
		assert(mPathMapping != nullptr);
		return mPathMapping->mOutputPath;
	}


	std::vector<std::string> ProjectInfo::getModuleDirectories() const
	{
		std::vector<std::string> dirs;
		auto projectDir = getProjectDir();

		if (mPathMapping->mModulePaths.empty())
		{
			nap::Logger::error("No module paths specified in path mapping: %s", mPathMapping->getFilename().c_str());
			return {};
		}

		// make all paths absolute
		for (const auto& p : mPathMapping->mModulePaths)
		{
			dirs.emplace_back(utility::forceSeparator(utility::isAbsolutePath(p) ? p :
				utility::joinPath({ projectDir, p })));
		}
		return dirs;
	}


	bool ProjectInfo::isEditorMode() const
	{
		return mContext == EContext::Editor;
	}


	std::string ProjectInfo::getFilename() const
	{
		assert(!mFilename.empty());
		return mFilename;
	}


	std::string ProjectInfo::getProjectDir() const
	{
		assert(!mProjectDir.empty());
		return mProjectDir;
	}


	std::string ProjectInfo::getDataFile() const
	{
		if (mDefaultData.empty()) return {};
		return utility::joinPath({getProjectDir(), mDefaultData});
	}


	std::string ProjectInfo::getDataDirectory() const
	{
		return utility::joinPath(
			{
				getProjectDir(),
				getDataFile().empty() ? projectinfo::dataDir : utility::getFileDir(mDefaultData)
			});
	}


	std::string ProjectInfo::getIniDir() const
	{
		return utility::stringFormat("%s/%s", getProjectDir().c_str(), projectinfo::iniDirectory);
	}


	std::string ProjectInfo::getIniFilePath(const std::string& name) const
	{
		return utility::stringFormat("%s/%s%s", getIniDir().c_str(), name.c_str(), projectinfo::iniExtension);
	}


	const PathMapping& ProjectInfo::getPathMapping() const
	{
		// Expected to exist when created
		assert(mPathMapping);
		return *mPathMapping;
	}


	void ProjectInfo::patchPath(std::string& path, const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		path = utility::forceSeparator(path);
		utility::namedFormat(path, getTemplateValues(additionalValues));
	};


	void ProjectInfo::patchPaths(std::vector<std::string>& paths, const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		for (auto& path : paths)
		{
			patchPath(path, additionalValues);
		}
	}


	static nap::rtti::Factory& getFactory()
	{
		static nap::rtti::Factory factory;
		return factory;
	}


	std::unique_ptr<nap::ProjectInfo> ProjectInfo::clone() const
	{
		auto cloned_info = rtti::cloneObject<ProjectInfo>(*this, getFactory());
		assert(cloned_info != nullptr);
		cloned_info->mFilename = mFilename;
		cloned_info->mContext = mContext;
		cloned_info->mProjectDir = mProjectDir;
		cloned_info->mRoot = mRoot;
		cloned_info->mPathMapping = rtti::cloneObject<PathMapping>(*mPathMapping, getFactory());
		assert(cloned_info->mPathMapping != nullptr);
		return cloned_info;
	}


	bool ProjectInfo::init(const std::string fileName, ProjectInfo::EContext context, nap::utility::ErrorState& error)
	{
		// Resolve project directory
		mContext = context; mFilename = fileName;
		mProjectDir = nap::utility::getFileDir(nap::utility::getAbsolutePath(fileName));
		if (!error.check(!mProjectDir.empty(), "Unable to resolve project directory from file: %s",
			fileName.c_str()))
			return false;

		// Load path mapping (relative to the app.json file)
		auto path_mapping_file_name = utility::forceSeparator(utility::joinPath({ mProjectDir, mPathMappingFile }));
		mPathMapping = nap::rtti::getObjectFromJSONFile<nap::PathMapping>(path_mapping_file_name,
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			getFactory(),
			error);

		// Ensure path loading path mapping succeeded
		if (!error.check(mPathMapping != nullptr, "Failed to load path mapping: %s",
			path_mapping_file_name.c_str()))
			return false;

		// Resolve the NAP root
		auto nap_root = utility::joinPath(
			{
				utility::getExecutableDir(), context == ProjectInfo::EContext::Editor ?
					mPathMapping->mNapkinExeToRoot :
					mPathMapping->mProjectExeToRoot
			});
		mRoot = utility::getAbsolutePath(nap_root);

		// Ensure root is valid
		if (!error.check(!mRoot.empty(), "Unable to resolve NAP root directory"))
			return false;

		// Resolve module paths
		patchPaths(mPathMapping->mModulePaths);

		// Resolve build output
		patchPath(mPathMapping->mOutputPath);

		// Resolve service config
		patchPath(mServiceConfigFilename);

		return true;
	}


	std::unordered_map<std::string, std::string> ProjectInfo::getTemplateValues(const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		// Default templates
		static const std::unordered_map<std::string, std::string> default_values(
		{
			{"ROOT",			getNAPRootDir()},
			{"BUILD_ARCH",		sBuildArch},
			{"BUILD_CONFIG",	sBuildConf},
			{"BUILD_TYPE",		sBuildType},
			{"PROJECT_DIR",		getProjectDir()},
		});

		// As EXE_DIR is the project executable directory the editor won't be aware of this and shouldn't try and populate it
		auto templates = default_values;
		if (mContext == EContext::Application)
			templates["EXE_DIR"] = utility::getExecutableDir();

		// Add additional (requested) ones
		for (const auto& add_value : additionalValues)
		{
			auto inserted = templates.emplace(add_value);
			if (!inserted.second)
				nap::Logger::warn("%s: Duplicate template value: %s", add_value.first.c_str(), add_value.second.c_str());
		}
		return templates;
	}


	bool ProjectInfo::hasServiceConfigFile() const
	{
		return !mServiceConfigFilename.empty();
	}


	std::string ModuleInfo::getFilename() const
	{
		return mFilename;
	}


	std::string ModuleInfo::getDirectory() const
	{
		return utility::getFileDir(getFilename());
	}


	const ProjectInfo& ModuleInfo::getProjectInfo() const
	{
		// Expected to exist when created
		assert(mProjectInfo);
		return *mProjectInfo;
	}
}
