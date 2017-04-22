#include "directorywatcher.h"
#include "assert.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#undef min
#undef max

namespace nap
{

	/**
	* Internal data container to hide windows internals from the header.
	*/
	struct DirectoryWatcher::PImpl
	{
		HANDLE					mDirectoryToMonitor;		// Handle to directory that is monitored
		HANDLE					mOverlappedEvent;			// Manual reset event that is signaled when data is available
		OVERLAPPED				mOverlapped;				// Overlapped struct so that I/O calls are non-blocking
		FILE_NOTIFY_INFORMATION mNotifications[1024];		// Output struct with file notification info
	};


	/**
	* Installs monitor: opens directory, creates event, starts directory scan.
	*/
	DirectoryWatcher::DirectoryWatcher() :
		mPImpl(std::make_unique<DirectoryWatcher::PImpl>())
	{
		char current_directory[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, current_directory);

		// Open directory 
		mPImpl->mDirectoryToMonitor = CreateFileA(current_directory, FILE_LIST_DIRECTORY, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		assert(mPImpl->mDirectoryToMonitor != INVALID_HANDLE_VALUE);

		// Create event
		mPImpl->mOverlappedEvent = CreateEvent(NULL, true, false, NULL);

		// Start initial query
		std::memset(&mPImpl->mNotifications, 0, sizeof(mPImpl->mNotifications));
		std::memset(&mPImpl->mOverlapped, 0, sizeof(mPImpl->mOverlapped));
		mPImpl->mOverlapped.hEvent = mPImpl->mOverlappedEvent;

		DWORD result = ReadDirectoryChangesW(mPImpl->mDirectoryToMonitor, (LPVOID)&mPImpl->mNotifications, sizeof(mPImpl->mNotifications), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &mPImpl->mOverlapped, NULL);
		assert(result != 0 || GetLastError() == ERROR_IO_PENDING);
	}


	/**
	* Cleanup
	*/
	DirectoryWatcher::~DirectoryWatcher()
	{
		CloseHandle(mPImpl->mDirectoryToMonitor);
		CloseHandle(mPImpl->mOverlappedEvent);
	}



	/**
	* Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	* multiple updates.
	*/
	bool DirectoryWatcher::update(std::string& modifiedFile)
	{
		bool did_update = false;

		// Overlapped event is signaled when data is available. By passing 0 we never block but return immediately (so this is how we poll non-blocking)
		DWORD result = WaitForSingleObject(mPImpl->mOverlappedEvent, 0);
		if (result == WAIT_OBJECT_0)
		{
			DWORD dwBytesReturned = 0;
			result = GetOverlappedResult(mPImpl->mDirectoryToMonitor, &mPImpl->mOverlapped, &dwBytesReturned, FALSE);
			if (result != 0)
			{
				// Copy from wide string to narrow string
				modifiedFile.resize(mPImpl->mNotifications[0].FileNameLength / 2);
				wcstombs(&modifiedFile[0], mPImpl->mNotifications->FileName, mPImpl->mNotifications[0].FileNameLength / 2);

				did_update = true;

				// This is a manual reset event, so we have to do ourselves
				ResetEvent(mPImpl->mOverlappedEvent);

				// Now start a new query
				std::memset(&mPImpl->mNotifications, 0, sizeof(mPImpl->mNotifications));
				std::memset(&mPImpl->mOverlapped, 0, sizeof(mPImpl->mOverlapped));
				mPImpl->mOverlapped.hEvent = mPImpl->mOverlappedEvent;

				result = ReadDirectoryChangesW(mPImpl->mDirectoryToMonitor, (LPVOID)&mPImpl->mNotifications, sizeof(mPImpl->mNotifications), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &mPImpl->mOverlapped, NULL);
				assert(result != 0 || GetLastError() == ERROR_IO_PENDING);
			}
		}

		return did_update;
	}

}
