#include "nap/directorywatcher.h"
#include <FileWatcher/FileWatcher.h>
#include <nap/logger.h>
#include <utility/fileutils.h>
#include <fstream>
#include <thread>

namespace nap {

    class DirectoryWatcher::PImpl : public FW::FileWatchListener {
    public:
        void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
                              FW::Action action) override
        {
            if (action == FW::Actions::Add || action == FW::Actions::Modified) {

                auto fname = dir + "/" + filename;

                nap::Logger::info("File changed: %s", fname.c_str());
                modifiedFiles.push_back(fname);
            }
        }

        std::vector<std::string> modifiedFiles;
        FW::FileWatcher fileWatcher;
        FW::WatchID watchID;
    };


    void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl* ptr) const
    { delete ptr; }


    DirectoryWatcher::DirectoryWatcher()
    {
        // PImpl instantiation using unique_ptr because we only want a unique snowflake
        mPImpl = std::unique_ptr<PImpl, PImpl_deleter>(new PImpl);

        std::string path = utility::getAbsolutePath(".");
        nap::Logger::info("Watching: %s", path.c_str());
        mPImpl->watchID = mPImpl->fileWatcher.addWatch(path, &(*mPImpl), true);

    }


    DirectoryWatcher::~DirectoryWatcher()
    {
        mPImpl->fileWatcher.removeWatch(mPImpl->watchID);
    }


    /**
     * Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
     * multiple updates.
     */
    bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
    {
        mPImpl->fileWatcher.update();

        if (mPImpl->modifiedFiles.empty())
            return false;

        modifiedFiles.insert(modifiedFiles.end(), mPImpl->modifiedFiles.begin(), mPImpl->modifiedFiles.end());

        mPImpl->modifiedFiles.clear();

        return true;
    }


}
