#include "projectinfo.h"
#include "core.h"
#include "logger.h"
#include "utility/fileutils.h"

RTTI_BEGIN_CLASS(nap::PathMapping)
	RTTI_PROPERTY("ProjectExeToRoot", &nap::PathMapping::mProjectExeToRoot, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NapkinExeToRoot", &nap::PathMapping::mNapkinExeToRoot, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ModulePaths", &nap::PathMapping::mModulePaths, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ServicesInfo)
		RTTI_PROPERTY("ServiceConfigurations", &nap::ServicesInfo::mServiceConfigurations, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ProjectInfo)
	RTTI_PROPERTY("Title", &nap::ProjectInfo::mTitle, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Version", &nap::ProjectInfo::mVersion, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Data", &nap::ProjectInfo::mDefaultData, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathMapping", &nap::ProjectInfo::mPathMappingFile, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ServiceConfig", &nap::ProjectInfo::mServicesInfoFile, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RequiredModules", &nap::ProjectInfo::mRequiredModules, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::ModuleInfo)
	RTTI_PROPERTY("RequiredModules", &nap::ModuleInfo::mRequiredModules, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WindowsDllSearchPaths", &nap::ModuleInfo::mLibSearchPaths, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	std::string ProjectInfo::getNAPRootDir() const
	{
		auto exeDir = nap::utility::getExecutableDir();
		auto napKinExeToRoot = mPathMapping->mNapkinExeToRoot;
		auto projectToRoot = mPathMapping->mProjectExeToRoot;
		return utility::joinPath({exeDir, mEditorMode ? napKinExeToRoot : projectToRoot});
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
			if (utility::isAbsolutePath(p))
				dirs.emplace_back(p);
			else
				dirs.emplace_back(utility::joinPath({projectDir, p}));
		}

		return dirs;
	}

	bool ProjectInfo::isEditorMode() const
	{
		return mEditorMode;
	}

	void ProjectInfo::setEditorMode(bool b)
	{
		mEditorMode = b;
	}

	std::string ProjectInfo::getFilename() const
	{
		return mFilename;
	}

	void ProjectInfo::setFilename(const std::string& filename)
	{
		mFilename = filename;
	}

	std::string ProjectInfo::getProjectDir() const
	{
		assert(!mFilename.empty());
		auto f = nap::utility::getAbsolutePath(mFilename);
		return nap::utility::getFileDir(f);
	}

	std::string ProjectInfo::getDefaultDataFile() const
	{
		if (mDefaultData.empty()) return {};
		return utility::joinPath({getProjectDir(), mDefaultData});
	}


	std::string ProjectInfo::dataDirectory() const
	{
		auto dataFile = getDefaultDataFile();
		if (dataFile.empty())
			return utility::joinPath({getProjectDir(), "data"});
		return utility::joinPath({getProjectDir(), utility::getFileDir(mDefaultData)});
	}

	const PathMapping& ProjectInfo::getPathMapping() const
	{
		// Expected to exist when created
		assert(mPathMapping);
		return *mPathMapping;
	}

	bool ProjectInfo::patchPath(std::string& path, const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		utility::replaceTemplateVariable(path, getTemplateValues(additionalValues));
		return true;
	}

	bool ProjectInfo::patchPaths(std::vector<std::string>& paths, const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		utility::replaceTemplateVariables(paths, getTemplateValues(additionalValues));
		return true;
	}

	std::unordered_map<std::string, std::string> ProjectInfo::getTemplateValues(const std::unordered_map<std::string, std::string>& additionalValues) const
	{
		std::unordered_map<std::string, std::string> values({
			{"ROOT", getNAPRootDir()},
			{"BUILD_CONFIG", sBuildConf},
			{"BUILD_TYPE", sBuildType},
			{"PROJECT_DIR", getProjectDir()},
		});

		// As EXE_DIR is the project executable directory the editor won't be aware of this and shouldn't
		// try and populate it
		if (!isEditorMode())
			values["EXE_DIR"] = utility::getExecutableDir();

		// TODO: For our own safety: verify unique value insertion
		values.insert(additionalValues.begin(), additionalValues.end());

		return values;
	}

	const ProjectInfo& ModuleInfo::getProjectInfo() const
	{
		// Expected to exist when created
		assert(mProjectInfo);
		return *mProjectInfo;
	}
}
