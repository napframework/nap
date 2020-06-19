#include "projectinfo.h"
#include "logger.h"
#include "utility/fileutils.h"

RTTI_BEGIN_CLASS(nap::PathMapping)
	RTTI_PROPERTY("ProjectExeToRoot", &nap::PathMapping::mProjectExeToRoot, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NapkinExeToRoot", &nap::PathMapping::mNapkinExeToRoot, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ModulePaths", &nap::PathMapping::mModulePaths, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS



RTTI_BEGIN_CLASS(nap::ProjectInfo)
	RTTI_PROPERTY("Title", &nap::ProjectInfo::mTitle, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Version", &nap::ProjectInfo::mVersion, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Data", &nap::ProjectInfo::mDefaultData, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathMapping", &nap::ProjectInfo::mPathMappingFile, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RequiredModules", &nap::ProjectInfo::mModuleNames, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::ModuleInfo)
	RTTI_PROPERTY("Dependencies", &nap::ModuleInfo::mDependencies, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	std::string ProjectInfo::getDirectory() const
	{
		assert(!mFilename.empty());
		auto f = nap::utility::getAbsolutePath(mFilename);
		return nap::utility::getFileDir(f);
	}

	std::vector<std::string> ProjectInfo::getModuleDirectories() const
	{
		std::vector<std::string> dirs;
		auto projectDir = getDirectory();

		if (mPathMapping->mModulePaths.empty())
		{
			nap::Logger::error("No module paths specified in path mapping: %s",
							   mPathMapping->mFilename.c_str());
			return {};
		}

		// make all paths absolute
		for (const auto& p : mPathMapping->mModulePaths)
		{
			if (utility::isAbsolutePath(p))
				dirs.emplace_back(p);
			else
				dirs.emplace_back(utility::stringFormat("%s/%s", projectDir.c_str(), p.c_str()));
		}

		return dirs;
	}

	std::string ProjectInfo::getDefaultDataFile() const
	{
		if (mDefaultData.empty())
			return {};
		return getDirectory() + "/" + mDefaultData;
	}


	std::string ProjectInfo::dataDirectory() const
	{
		return getDirectory() + "/" + utility::getFileDir(mDefaultData);
	}
}
