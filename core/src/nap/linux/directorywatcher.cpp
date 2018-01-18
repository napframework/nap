#include "nap/directorywatcher.h"
#include "nap/linux/FileWatcher/FileWatcher.h"

#include <fstream>
#include <nap/logger.h>
#include <thread>
#include <utility/fileutils.h>
#include "nap/datapathmanager.h"

namespace nap
{
	/**
	 * Directorywatcher
	 */
	class DirectoryWatcher::PImpl : public FW::FileWatchListener
	{
	public:
		void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
							  FW::Action action) override
		{
			auto fname = dir + "/" + filename;
			switch (action)
			{
			case FW::Actions::Delete:
				nap::Logger::info("DEL %s", fname.c_str());
				break;
			case FW::Actions::Add:
				nap::Logger::info("ADD %s", fname.c_str());
				modifiedFiles.push_back(fname);
				break;
			case FW::Actions::Modified:
				nap::Logger::info("MOD %s", fname.c_str());
				modifiedFiles.push_back(fname);
				break;
			}
		}

		std::vector<std::string> modifiedFiles;
		FW::FileWatcher fileWatcher;
		FW::WatchID watchID;
	};


	void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl* ptr) const { delete ptr; }


	DirectoryWatcher::DirectoryWatcher(std::string projectDataPath)
	{
		// PImpl instantiation using unique_ptr because we only want a unique snowflake
		mPImpl = std::unique_ptr<PImpl, PImpl_deleter>(new PImpl);
		
		nap::Logger::debug("Watching directory: %s", projectDataPath.c_str());
		mPImpl->watchID = mPImpl->fileWatcher.addWatch(projectDataPath, &(*mPImpl), true);
	}


	DirectoryWatcher::~DirectoryWatcher() { mPImpl->fileWatcher.removeWatch(mPImpl->watchID); }


	/**
	 * Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	 * multiple updates.
	 */
	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
		mPImpl->fileWatcher.update();

		if (mPImpl->modifiedFiles.empty())
			return false;

		for (auto filename : mPImpl->modifiedFiles)
		{
			if (utility::fileExists(filename))
				modifiedFiles.push_back(filename);
		}

		mPImpl->modifiedFiles.clear();

		return true;
	}
}
