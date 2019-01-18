#include "nap/directorywatcher.h"
#include "nap/linux/FileWatcher/FileWatcher.h"

#include <fstream>
#include <nap/logger.h>
#include <thread>
#include <utility/fileutils.h>

#include <unistd.h>
#include <linux/limits.h>

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
		std::string currentPath;				// path to the current working directory
		FW::FileWatcher fileWatcher;
		FW::WatchID watchID;
	};


	void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl* ptr) const { delete ptr; }


	DirectoryWatcher::DirectoryWatcher()
	{
		// TODO ANDROID Clean this up!
#ifdef ANDROID
		return;
#endif		
		// PImpl instantiation using unique_ptr because we only want a unique snowflake
		mPImpl = std::unique_ptr<PImpl, PImpl_deleter>(new PImpl);

		// Retrieve the path to the current working dir
		char buffer[PATH_MAX];
		std::string path = std::string(getcwd(buffer, PATH_MAX));
		mPImpl->currentPath = path;

		nap::Logger::debug("Watching directory: %s", path.c_str());
		mPImpl->watchID = mPImpl->fileWatcher.addWatch(path, &(*mPImpl), true);
	}


	DirectoryWatcher::~DirectoryWatcher() { 
#ifdef ANDROID
		return;
#endif				
		mPImpl->fileWatcher.removeWatch(mPImpl->watchID); 
	}


	/**
	 * Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	 * multiple updates.
	 */
	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
#ifdef ANDROID
		return false;
#endif
		mPImpl->fileWatcher.update();

		if (mPImpl->modifiedFiles.empty())
			return false;

		std::string comparable_watched_path = utility::toComparableFilename(mPImpl->currentPath);

		for (auto filename : mPImpl->modifiedFiles)
		{
			std::string comparable_modified_file = utility::toComparableFilename(filename);
			
			// Check if the watched path is found at the start if the modified file's path
			auto pos = comparable_modified_file.find(comparable_watched_path + "/");
			assert(pos != std::string::npos);

			// Strip the watched path from the start
			comparable_modified_file.erase(0, mPImpl->currentPath.size() + 1);
			modifiedFiles.emplace_back(comparable_modified_file);
		}

		mPImpl->modifiedFiles.clear();

		return true;
	}
}
