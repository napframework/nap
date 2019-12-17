#pragma once
#include <rtti/typeinfo.h>

namespace nap
{
	class ProjectInfo
	{
		RTTI_ENABLE()
	public:
		std::string mFilename;
		std::string mTitle;
		std::string mVersion;
		std::vector<std::string> mModuleNames;
		std::vector<std::string> mModuleDirs;

		std::string getDirectory() const;
		std::vector<std::string> getModuleDirectories() const;

		bool load(const std::string& filename, utility::ErrorState& err);

	};

	class ModuleInfo
	{
		RTTI_ENABLE()
	public:
		std::string mFilename;
		std::vector<std::string> mDependencies;

		bool load(const std::string& filename, utility::ErrorState& err);
	};
}