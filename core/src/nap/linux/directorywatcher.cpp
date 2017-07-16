#include "nap/directorywatcher.h"

namespace nap {

    struct DirectoryWatcher::PImpl {
    };


    void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl *ptr) const { delete ptr; }


    DirectoryWatcher::DirectoryWatcher() {

    }


    DirectoryWatcher::~DirectoryWatcher() {
    }


    /**
     * Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
     * multiple updates.
     */
    bool DirectoryWatcher::update(std::vector<std::string> &modifiedFiles) {
        // TODO: Implement
        return false;
    }
}
