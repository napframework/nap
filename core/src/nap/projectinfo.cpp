#include "projectinfo.h"
#include "logger.h"
#include "utility/fileutils.h"

RTTI_BEGIN_CLASS(nap::PathMapping)
	RTTI_PROPERTY("ProjectExeToRoot", &nap::PathMapping::mProjectExeToRoot, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NapkinExeToRoot", &nap::PathMapping::mNapkinExeToRoot, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ModulePaths", &nap::PathMapping::mModulePaths, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS



RTTI_BEGIN_CLASS(nap::ProjectInfo)
	RTTI_PROPERTY("Title", &nap::ProjectInfo::mTitle, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Version", &nap::ProjectInfo::mVersion, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Data", &nap::ProjectInfo::mDefaultData, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathMapping", &nap::ProjectInfo::mPathMappingFile, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RequiredModules", &nap::ProjectInfo::mRequiredModules, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::ModuleInfo)
	RTTI_PROPERTY("RequiredModules", &nap::ModuleInfo::mRequiredModules, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WindowsDllSearchPaths", &nap::ModuleInfo::mLibSearchPaths, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

//  Somehow, msvc cannot find this implementation when separating the declaration
//	std::string ProjectInfo::getProjectDir() const
//	{
//		assert(!mFilename.empty());
//		auto f = nap::utility::getAbsolutePath(mFilename);
//		return nap::utility::getFileDir(f);
//	}

	std::string ProjectInfo::getNAPRootDir() const
	{
		return utility::joinPath({nap::utility::getExecutableDir(),
								  mEditorMode ? mPathMapping->mNapkinExeToRoot : mPathMapping->mProjectExeToRoot});
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

//  Somehow, msvc cannot find this implementation when separating the declaration
//	std::string ProjectInfo::getDefaultDataFile() const
//	{
//		if (mDefaultData.empty())
//			return {};
//		return utility::joinPath({getProjectDir(), mDefaultData});
//	}

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
