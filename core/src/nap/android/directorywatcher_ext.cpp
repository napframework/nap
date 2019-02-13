#include <nap/directorywatcher.h>

// For now we're not doing any directory watching on Android. If we ever support files outside of the APK on 
// Android which may be edited externally this will need to be revisited.

namespace nap
{
	DirectoryWatcher::DirectoryWatcher() {}

	DirectoryWatcher::~DirectoryWatcher() {}

    void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl* ptr) const { }

	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
		return false;
	}
}
